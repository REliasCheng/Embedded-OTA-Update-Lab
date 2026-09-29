#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "flash_drv.h"
#include "store_app.h"

typedef struct {
  uint16_t  magicCode;
	
	/* 添加配置参数开始 */
  uint8_t modbusAddr;
	char softwareVersion[10];

	/* 添加配置参数结束 */
	
	uint8_t crcVal;
} SysParam_t;


#define MAGIC_CODE     0x5A5A
static const SysParam_t g_sysParamDefault =
{
  .magicCode = MAGIC_CODE,
	.softwareVersion = APP_VERSION,
//	.modbusAddr = 1
};

static SysParam_t g_sysParamCurrent;


/* 产品配置相关参数 */
#define SYSPARAM_MAX_SIZE              (PARAMETER_SIZE / 2)
#define SYSPARAM_START_ADDR            PARAMETER_ADDR_IN_FLASH
#define SYSPARAM_BACKUP_START_ADDR     (SYSPARAM_START_ADDR + SYSPARAM_MAX_SIZE)

static uint8_t CalcCrc8(uint8_t *buf, uint32_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t byte = 0; byte < len; byte++)
    {
        crc ^= (buf[byte]);
        for (uint8_t i = 8; i > 0; --i)
        {
					if (crc & 0x80)
					{
						crc = (crc << 1) ^ 0x31;
					}
					else 
					{	
						crc = (crc<<1);
					}
				}
    }
    return crc;
}

static bool ReadDataWithCheck(uint32_t readAddr, uint8_t *pBuffer, uint32_t numToRead)
{
	if (!FlashRead(readAddr, pBuffer, numToRead))
	{
		return false;
	}
	uint8_t crcVal = CalcCrc8(pBuffer, numToRead - 1);
	if (crcVal != pBuffer[numToRead - 1])
	{
		return false;
	}
	return true;
}

static bool ReadSysParam(SysParam_t *sysParam)
{
	uint16_t sysParamLen = sizeof(SysParam_t);
	
	if (ReadDataWithCheck(SYSPARAM_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return true;
	}
	if (ReadDataWithCheck(SYSPARAM_BACKUP_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return true;
	}
	return false;
}

static bool WriteDataWithCheck(uint32_t writeAddr, uint8_t *pBuffer, uint32_t numToWrite)
{
	pBuffer[numToWrite - 1] = CalcCrc8(pBuffer, numToWrite - 1);
	if (!FlashErase(writeAddr, numToWrite))
	{
		return false;
	}
	if (!FlashWrite(writeAddr, pBuffer, numToWrite))
	{
		return false;
	}
	return true;
}
	
static bool WriteSysParam(SysParam_t *sysParam)
{
	uint16_t sysParamLen = sizeof(SysParam_t);
	if (sysParamLen > SYSPARAM_MAX_SIZE)
	{
		return false;
	}
	if (!WriteDataWithCheck(SYSPARAM_START_ADDR, (uint8_t *)sysParam, sysParamLen))
	{
		return false;
	}
	
	WriteDataWithCheck(SYSPARAM_BACKUP_START_ADDR, (uint8_t *)sysParam, sysParamLen);
	
	return true;
}

void InitSysParam(void)
{
	SysParam_t sysParam;
	
	// 读取Flash指定位置, 如果读到了数据，使用
	if (ReadSysParam(&sysParam) && sysParam.magicCode == MAGIC_CODE)
	{	
		g_sysParamCurrent = sysParam;
		return;
	}
	// 使用默认参数
	g_sysParamCurrent = g_sysParamDefault;
}

bool SetModbusParam(uint8_t addr)
{
	if (addr == g_sysParamCurrent.modbusAddr)
	{
		return true;
	}
	
	SysParam_t sysParam = g_sysParamCurrent;
//	sysParam.modbusAddr = addr;
	
	g_sysParamCurrent = sysParam;
	return true;
}

bool SetSoftwareVersionParam(char *version)
{
	if (version == NULL)
	{
		return false;
	}
	memset(g_sysParamCurrent.softwareVersion, 0, sizeof(g_sysParamCurrent.softwareVersion));
	SysParam_t sysParam = g_sysParamCurrent;
	strcpy(sysParam.softwareVersion, version);

	if (!WriteSysParam(&sysParam))
	{
		return false;
	}

	g_sysParamCurrent = sysParam;
	return true;
}

void GetSoftwareVersionParam(char *version)
{
	if (version == NULL)
	{
		return;
	}
	strcpy(version, g_sysParamCurrent.softwareVersion);
}

void SetUpdateVerFlag(void)
{
	uint16_t flag = NEED_UPDATE_VERSION_FLAG;
	FlashErase(UPDATE_INFO_ADDR_IN_FLASH, 2);

	FlashWrite(UPDATE_INFO_ADDR_IN_FLASH, (uint8_t *)&flag, 2);
}
