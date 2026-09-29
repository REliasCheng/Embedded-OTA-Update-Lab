#ifndef __BSP_BATTERY_LEDS_H__
#define __BSP_BATTERY_LEDS_H__

#include "gd32f4xx.h"
//#include "gd32f4xx_rcu.h"

#define LED1    1
#define LED2    2
#define LED3    3
#define LED4    4



void Battery_Leds_init();

// 点亮某个灯
void Battery_Leds_turn_on(uint8_t index);

// 熄灭某个灯
void Battery_Leds_turn_off(uint8_t index);

// ------------------------------ 业务相关函数
// 开始充电
void Battery_Leds_start(uint8_t power); //[0,4]

// 循环调用的函数
void Batter_Leds_loop();

// 结束充电
void Battery_Leds_stop();

// 更新电量
void Battery_Leds_update(uint8_t power);

#endif