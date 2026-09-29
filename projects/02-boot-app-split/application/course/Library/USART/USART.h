#ifndef __USART_H__
#define __USART_H__

#include "gd32f4xx.h"
#include "USART_config.h"

#if USE_USART0
#include "USART0.h"
#endif

#if USE_USART1
#include "USART1.h"
#endif

void USART_init();

#endif