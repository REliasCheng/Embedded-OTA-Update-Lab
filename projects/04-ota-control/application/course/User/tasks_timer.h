#ifndef __TASKS_TIMER_H__
#define __TASKS_TIMER_H__

#include "gd32f4xx.h"


void tasks_timer_init();

// 0x00000000 -> 0xFFFFFFFF
uint64_t task_timer_get_tick();

#endif