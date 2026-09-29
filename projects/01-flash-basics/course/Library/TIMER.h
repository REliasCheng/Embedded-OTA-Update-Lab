#ifndef __TIMER_H__
#define __TIMER_H__

#include "gd32f4xx.h"
#include "TIMER_config.h"

void TIMER_init();

/**********************************************************
 * @brief 更新指定Timer的分频系数和周期
 * @param t_prescaler 分频系数[1, 65536]
 * @param t_period    周期计数值[1, 65536]
 * @return 
 **********************************************************/
void TIMER_period_update(uint32_t timer, uint16_t t_prescaler, uint32_t t_period);

/**********************************************************
 * @brief 更新PWM指定通道的占空比
 * @param duty 占空比数值 [0, 100]
 **********************************************************/
void TIMER_channel_update(uint32_t timer, uint16_t channel, float duty);

#endif