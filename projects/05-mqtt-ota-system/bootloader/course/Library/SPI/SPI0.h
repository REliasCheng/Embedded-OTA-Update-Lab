#ifndef __SPI0_H__
#define __SPI0_H__

#include "gd32f4xx.h"
#include "SPI_config.h"

#define SCL(bit)    gpio_bit_write(SPI0_SCL_PORT,SPI0_SCL_PIN, bit ? SET : RESET)//SCL

#define MOSI(bit)   gpio_bit_write(SPI0_MOSI_PORT,SPI0_MOSI_PIN,bit ? SET : RESET)//MOSI

#define MISO()      gpio_input_bit_get(SPI0_MISO_PORT,SPI0_MISO_PIN)// MISO

void SPI0_init(void);

// 写数据
void SPI0_write(uint8_t dat);

// 读数据
uint8_t SPI0_read();

// 写数据&读数据
uint8_t SPI0_write_read(uint8_t dat);

#endif