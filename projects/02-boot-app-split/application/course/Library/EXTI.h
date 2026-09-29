#ifndef __EXTI_H__
#define __EXTI_H__

#include "gd32f4xx.h"
#include "EXTI_config.h"

void EXTI_init();

void EXTI_soft_trig(exti_line_enum linex);



/*!
    \brief    	外部中断触发函数
    \param[in]  linex: EXTI line number, refer to exti_line_enum
                only one parameter can be selected which is shown as below:
    \arg        EXTI_x (x=0..22): EXTI line x
*/			

//#if USE_EXTI_0 || USE_EXTI_1 || USE_EXTI_2 || USE_EXTI_3 || \
//		USE_EXTI_4 || USE_EXTI_5 || USE_EXTI_6 || USE_EXTI_7 ||	\
//		USE_EXTI_8 || USE_EXTI_9 || USE_EXTI_10|| USE_EXTI_11|| \
//		USE_EXTI_12|| USE_EXTI_13|| USE_EXTI_14|| USE_EXTI_15 
//		
//extern void EXTI_on_trig(exti_line_enum linex);

//#endif

#if USE_EXTI_0
void EXTI0_on_trig();
#endif
#if USE_EXTI_1
void EXTI1_on_trig();
#endif
#if USE_EXTI_2
void EXTI2_on_trig();
#endif

#if USE_EXTI_3
void EXTI3_on_trig();
#endif

#if USE_EXTI_4
void EXTI4_on_trig();
#endif

#if USE_EXTI_5
void EXTI5_on_trig();
#endif

#if USE_EXTI_6
void EXTI6_on_trig();
#endif

#if USE_EXTI_7
void EXTI7_on_trig();
#endif

#if USE_EXTI_8
void EXTI8_on_trig();
#endif

#if USE_EXTI_9
void EXTI9_on_trig();
#endif

#if USE_EXTI_10
void EXTI10_on_trig();
#endif

#if USE_EXTI_11
void EXTI11_on_trig();
#endif

#if USE_EXTI_12
void EXTI12_on_trig();
#endif

#if USE_EXTI_13
void EXTI13_on_trig();
#endif

#if USE_EXTI_14
void EXTI14_on_trig();
#endif

#if USE_EXTI_15
void EXTI15_on_trig();
#endif

#endif