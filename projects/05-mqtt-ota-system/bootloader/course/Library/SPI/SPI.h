#ifndef __SPI_H__
#define __SPI_H__

#include "gd32f4xx.h"
#include "SPI_config.h"


#if USE_SPI0
#include "SPI0.h"
#endif

#if USE_SPI1
#include "SPI1.h"
#endif

#if USE_SPI2
#include "SPI2.h"
#endif

void SPI_init();


#endif