#include "bsp_servo.h"
#include "TIMER.h"

void Servo_init(){

}

/**********************************************************
 * @brief 设置舵机角度
 * @param angle 舵机角度 [0, 180]

[0, 180] -> [500, 2500] * 100 / 20000

 **********************************************************/
void Servo_set_angle(float angle){
  float duty = (500 + (angle / 180) * 2000) * 100.0f / 20000;
  TIMER_channel_update(TIMER3, TIMER_CH_3, duty);
}