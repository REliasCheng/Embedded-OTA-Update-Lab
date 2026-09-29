#ifndef _BSP_W25Qxx_H__
#define _BSP_W25Qxx_H__

#include "gd32f4xx.h"
#include "SPI.h"

void W25Qxx_init_config(void);
uint16_t W25Qxx_readID(void);
void W25Qxx_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte);
void W25Qxx_read(uint8_t* buffer,uint32_t read_addr,uint16_t read_length) ;


/******************************************************************
 * 函 数 名 称：spi_read_write_byte
 * 函 数 说 明：硬件SPI的读写
 * 函 数 形 参：dat=发送的数据
 * 函 数 返 回：读取到的数据
 * 作       者：LC
 * 备       注：无
******************************************************************/
#define spi_read_write_byte(dat)	SPI4_read_write(dat)

#endif
