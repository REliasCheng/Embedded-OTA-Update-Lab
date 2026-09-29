#ifndef __BSP_BUZZER_H__
#define __BSP_BUZZER_H__

#include "gd32f4xx.h"

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t

void Buzzer_init();

// 通过音符的索引播放声音
void Buzzer_beep(u16 hz_val_index);
	
// 通过指定频率播放声音
void Buzzer_play(u16 hz_val);

void Buzzer_stop();

#endif