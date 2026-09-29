#ifndef __SPI_CONFIG_H__
#define __SPI_CONFIG_H__

#include "gd32f4xx.h"

// -------------------------------------------
#define USE_SPI0    1
#define USE_SPI1    0
#define USE_SPI2    0
#define USE_SPI3    0


// -------------------------------------------

// ======================= SPI0 =====================
#if USE_SPI0

#define SPI0_SOFT        1  // 0硬实现，1软实现

#define SPI0_SCL_RCU     RCU_GPIOA
#define SPI0_SCL_PORT    GPIOA
#define SPI0_SCL_PIN     GPIO_PIN_5
        
#define SPI0_MOSI_RCU    RCU_GPIOA
#define SPI0_MOSI_PORT   GPIOA
#define SPI0_MOSI_PIN    GPIO_PIN_7

#define SPI0_MISO_RCU    RCU_GPIOA
#define SPI0_MISO_PORT   GPIOA
#define SPI0_MISO_PIN    GPIO_PIN_6

// 时钟极性 Clock Polarity
#define SPI0_CPOL  1
// 时钟相位 Clock Phase
#define SPI0_CPHA  1

#endif

#endif

