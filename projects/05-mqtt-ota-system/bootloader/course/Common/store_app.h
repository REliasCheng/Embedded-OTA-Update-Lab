#ifndef _STORE_APP_H_
#define _STORE_APP_H_

#include <stdint.h>
#include <stdbool.h>

/* OTA相关参数  0x80000
start										  total
0x8000000                 0x80000 = (512K)

(512K - 32K - 2K - 4K) / 2 = 237KByte

			Bootloader				APP_SIZE							   BACKUP_SIZE     	  	Update	Param
|-----------------|-------------------------|-------------------------|----|--------|
		  	32K    				   	237K								   	237K								  2K  		4K

								          APP								     BACKUP								UPDATE PARAM
																																			0xABCD
*/
#define FLASH_SIZE                        0x80000
#define BOOTLOADER_SIZE                   (32 * 1024) // 0x8000
#define UPDATE_INFO_SIZE                  (2 * 1024)
#define PARAMETER_SIZE                    (4 * 1024)
#define APP_SIZE                          ((FLASH_SIZE - BOOTLOADER_SIZE - UPDATE_INFO_SIZE - PARAMETER_SIZE) / 2)  //要保证大小不能是奇数
#define BACKUP_SIZE                       APP_SIZE
#define APP_ADDR_IN_FLASH                 (0x08000000 + BOOTLOADER_SIZE) // 0X8008000
#define BACKUP_ADDR_IN_FLASH              (APP_ADDR_IN_FLASH + APP_SIZE)
#define UPDATE_INFO_ADDR_IN_FLASH         (BACKUP_ADDR_IN_FLASH + BACKUP_SIZE)
#define PARAMETER_ADDR_IN_FLASH           (UPDATE_INFO_ADDR_IN_FLASH + UPDATE_INFO_SIZE)
#define NEED_UPDATE_VERSION_FLAG          0xABCD
#define FLASH_PAGE_SIZE										0x400			// 1024

#define APP_VERSION												"1.0"

bool SetModbusParam(uint8_t addr);
void InitSysParam(void);
void GetSoftwareVersionParam(char *version);
bool SetSoftwareVersionParam(char *version);
void SetUpdateVerFlag(void);

bool CheckNeedUpdate(void);
void ClearUpdateVerFlag(void);
#endif
