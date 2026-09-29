#ifndef __BSP_SERVO_H__
#define __BSP_SERVO_H__

#include "gd32f4xx.h"

void Servo_init();

/**********************************************************
 * @brief 设置舵机角度
 * @param angle 舵机角度 [0, 180]
 **********************************************************/
void Servo_set_angle(float angle);

#endif