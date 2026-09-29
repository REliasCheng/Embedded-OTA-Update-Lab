#ifndef __BSP_KEYS_H__
#define __BSP_KEYS_H__

#include "gd32f4xx.h"

void Keys_init();

void Keys_scan();

extern void Keys_on_keydown(uint8_t index);
extern void Keys_on_keyup(uint8_t index);

#endif