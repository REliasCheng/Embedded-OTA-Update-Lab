#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "gd32f4xx.h"
#include "fmc_operation.h"
#include "flash_drv.h"

#define FLASH_PAGE_SIZE  		   		 	0x1000       // 4K
#define FLASH_END_ADDRESS						0x0807FFFF   // 512K

/**
*******************************************************************
* @function 指定地址开始读出指定个数的数据
* @param    readAddr,读取地址
* @param    pBuffer,数组首地址
* @param    numToRead,要读出的数据个数
* @return
*******************************************************************
*/
bool FlashRead(uint32_t readAddr, uint8_t *pBuffer, uint32_t numToRead)
{
  if ((readAddr + numToRead) > FLASH_END_ADDRESS)
  {
    return false;
  }
	
	// 0x0800 4008

  uint32_t addr = readAddr;
  for (uint32_t i = 0; i < numToRead; i++)
  {
		// 直接寻址访问
    *pBuffer = *((uint8_t *)addr);
    addr = addr + 1;
    pBuffer++;
  }
  return true;
}

#define FMC_CLS_FLAG()	fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGSERR)
#define FMC_CLS_FLAG2()  fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
bool FlashWrite8bit(uint32_t writeAddr, uint8_t *pBuffer, uint32_t numToWrite)
{
  if ((writeAddr + numToWrite) > FLASH_END_ADDRESS)
  {
    return false;
  }
  uint16_t temp;

  fmc_state_enum  fmcState = FMC_READY;

  fmc_unlock();

  FMC_CLS_FLAG2();
  for (uint32_t i = 0; i < numToWrite; i++)
  {
    fmcState = fmc_byte_program(writeAddr, pBuffer[i]);
    if (fmcState != FMC_READY)
    {
      fmc_lock();
      return false;
    }

    writeAddr ++;
  }

  fmc_lock();
  return true;
}
/**
*******************************************************************
* @function 指定地址开始写入指定个数的数据
* @param    writeAddr,写入地址
* @param    pBuffer,数组首地址
* @param    numToWrite,要写入的数据个数
* @return
*******************************************************************
*/
bool FlashWrite16bit(uint32_t writeAddr, uint16_t *pBuffer, uint32_t numToWrite)
{
  if ((writeAddr + numToWrite) > FLASH_END_ADDRESS)
  {
    return false;
  }
  if (writeAddr % 2 == 1)   // 半字(2字节)写入，地址要对齐
  {
		// XXX 0x0800 0025 
		// √√√ 0x0800 0022
    return false;
  }
  uint16_t temp;

  fmc_state_enum  fmcState = FMC_READY;

  fmc_unlock();

  FMC_CLS_FLAG2();
  for (uint32_t i = 0; i < numToWrite; i++)
  {
    fmcState = fmc_halfword_program(writeAddr, pBuffer[i]);
    if (fmcState != FMC_READY)
    {
      fmc_lock();
      return false;
    }

    writeAddr += 2;
  }
  fmc_lock();
  return true;
}
/**
*******************************************************************
* @function 擦除从eraseAddr开始到eraseAddr + numToErase的扇区
* @param    eraseAddr,地址
* @param    numToErase,对应写入数据时的个数
* @return
*******************************************************************
*/
bool FlashErase(uint32_t eraseAddr, uint32_t numToErase) {
  if (numToErase == 0 || (eraseAddr + numToErase) > FLASH_END_ADDRESS)
  {
    return false;
  }
  fmc_sector_info_struct start_sector_info;
  fmc_sector_info_struct end_sector_info;
  uint32_t sector_num,i;
  /* unlock the flash program erase controller */
  fmc_unlock();
  /* clear pending flags */
  fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
  /* get the information of the start and end sectors */
  start_sector_info = fmc_sector_info_get(eraseAddr);
  end_sector_info = fmc_sector_info_get(eraseAddr + numToErase);
  /* erase sector */
  for(i = start_sector_info.sector_name; i <= end_sector_info.sector_name; i++) {
    sector_num = sector_name_to_number(i);
    if(FMC_READY != fmc_sector_erase(sector_num)) {
      goto erase_err;
    }
  }
  /* lock the flash program erase controller */
  fmc_lock();
  return true;

erase_err:
  /* lock the main FMC after the erase operation */
  fmc_lock();
  return false;
}
#if defined (GD32F425) || defined (GD32F427) || defined (GD32F470)
/**
*******************************************************************
* @function 擦除从eraseAddr开始到eraseAddr + numToErase的页
* @param    eraseAddr,地址
* @param    numToErase,对应写入数据时的个数
* @return
*******************************************************************
*/
bool FlashErasePage(uint32_t eraseAddr, uint32_t numToErase)
{
	if (numToErase == 0 || (eraseAddr + numToErase) > FLASH_END_ADDRESS)
	{
		return false;
	}

	uint8_t pageNum;
	uint8_t addrOffset = eraseAddr % FLASH_PAGE_SIZE; 	// mod运算求余在一页内的偏移，若eraseAddr是FLASH_PAGE_SIZE整数倍，运算结果为0

	fmc_state_enum fmcState = FMC_READY;
	fmc_unlock();

	if (numToErase > (FLASH_PAGE_SIZE - addrOffset))           // 跨页
	{
		FMC_CLS_FLAG();
		fmcState = fmc_page_erase(eraseAddr);           // 擦本页
		if (fmcState != FMC_READY)
		{
			goto erase_err;
		}

		eraseAddr += FLASH_PAGE_SIZE - addrOffset;   // 对齐到页地址
		numToErase -= FLASH_PAGE_SIZE - addrOffset;
		pageNum = numToErase / FLASH_PAGE_SIZE;

		while (pageNum--)
		{
			FMC_CLS_FLAG();
			fmcState = fmc_page_erase(eraseAddr);
			if (fmcState != FMC_READY)
			{
				goto erase_err;
			}
			eraseAddr += FLASH_PAGE_SIZE;
		}
		if (numToErase % FLASH_PAGE_SIZE != 0)
		{
			FMC_CLS_FLAG();
			fmcState = fmc_page_erase(eraseAddr);
			if (fmcState != FMC_READY)
			{
				goto erase_err;
			}
		}
	}
	else  // 没有跨页
	{
		FMC_CLS_FLAG();
		fmcState = fmc_page_erase(eraseAddr);
		if (fmcState != FMC_READY)
		{
			goto erase_err;
		}
	}
	/* lock the main FMC after the erase operation */
    fmc_lock();
	return true;

erase_err:
	/* lock the main FMC after the erase operation */
    fmc_lock();
	return false;
}
#endif

#define BUFFER_SIZE                   10
//#define FLASH_TEST_ADDRESS            0x0807F004
#define FLASH_TEST_ADDRESS            0x08004000
void FlashDrvTest(void)
{
  uint8_t bufferWrite[BUFFER_SIZE];
  uint8_t bufferRead[BUFFER_SIZE];

  printf("flash writing data：\n");
  for (uint16_t i = 0; i < BUFFER_SIZE; i++){
    bufferWrite[i] = i;
    printf("0x%02X ", bufferWrite[i]);
  }
  printf("\n开始擦除\n");

	// 指定空间开始位置，和要擦除的大小
  if (!FlashErase(FLASH_TEST_ADDRESS, BUFFER_SIZE)){
    printf("Flash写数据故障，请排查！\n");
    return;
  }
  printf("\n开始写入\n");

	// 写入数据
  if (!FlashWrite(FLASH_TEST_ADDRESS, bufferWrite, BUFFER_SIZE)){
    printf("Flash写数据故障，请排查！\n");
    return;
  }

  printf("开始读取\n");
  if (!FlashRead(FLASH_TEST_ADDRESS, bufferRead, BUFFER_SIZE)){
    printf("Flash读数据故障，请排查！\n");
    return;
  }
	
  for (uint16_t i = 0; i < BUFFER_SIZE; i++){
    if (bufferRead[i] != bufferWrite[i]) {
      printf("0x%02X ", bufferRead[i]);
      printf("Flash测试故障，请排查！\n");
      return;
    }
    printf("0x%02X ", bufferRead[i]);

  }
  printf("\nFlash测试通过！\n");
}
