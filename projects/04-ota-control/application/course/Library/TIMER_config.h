#ifndef __TIMER_CONFIG_H__
#define __TIMER_CONFIG_H__

#include "gd32f4xx.h"

#define	USE_TIMER_0			0
#define	USE_TIMER_1			0
#define	USE_TIMER_2			0
#define	USE_TIMER_3			0
#define	USE_TIMER_4			0
#define	USE_TIMER_5			0
#define	USE_TIMER_6			0
#define	USE_TIMER_7			0
#define	USE_TIMER_8			0
#define	USE_TIMER_9			0
#define	USE_TIMER_10		0
#define	USE_TIMER_11		0
#define	USE_TIMER_12		0
#define	USE_TIMER_13		0

#if	USE_TIMER_0

#define TM0_PRESCALER		1
#define TM0_FREQ				10000
#define TM0_PERIOD			SystemCoreClock / (TM0_PRESCALER * TM0_FREQ)

// PA8, PE9
#define TM0_CH0						RCU_GPIOE, GPIOE, GPIO_PIN_9, GPIO_AF_1	
// PA7, PB13, PE8
#define TM0_CH0_ON				RCU_GPIOE, GPIOE, GPIO_PIN_8, GPIO_AF_1	

//// PA9, PE11
//#define TM0_CH1					RCU_GPIOA, GPIOA, GPIO_PIN_8, GPIO_AF_1	
//// PB0_ON, PB14_ON, PE1_ON, PE10_ON
//#define TM0_CH1_ON				RCU_GPIOA, GPIOA, GPIO_PIN_7, GPIO_AF_1	

//// PA10, PE13
//#define TM0_CH2					RCU_GPIOA, GPIOA, GPIO_PIN_8, GPIO_AF_1	
//// PB1_ON, PB15_ON, PE12_ON
//#define TM0_CH2_ON				RCU_GPIOA, GPIOA, GPIO_PIN_7, GPIO_AF_1	

//// PA11, PE14
//#define TM0_CH3					RCU_GPIOA, GPIOA, GPIO_PIN_8, GPIO_AF_1	

void TIMER_0_channel_update(uint16_t channel, float duty);
#endif

#if	USE_TIMER_1

#define TM1_PRESCALER		10
#define TM1_FREQ				1000
#define TM1_PERIOD			SystemCoreClock / (TM1_PRESCALER * TM1_FREQ)


// PA0, PA5, PA15, PB8
//#define TM1_CH0					RCU_GPIOA, GPIOA, GPIO_PIN_5	

// PA1,PB3,PB9
#define TM1_CH1					RCU_GPIOB, GPIOB, GPIO_PIN_9	

#endif

#if	USE_TIMER_2

#define TM2_PRESCALER		10
#define TM2_FREQ				1000
#define TM2_PERIOD			SystemCoreClock / (TM2_PRESCALER * TM2_FREQ)

// PA6, PB4, PC6
//#define TM2_CH0					RCU_GPIOC, GPIOC, GPIO_PIN_6
// PA7, PB5, PC7
//#define TM2_CH1					RCU_GPIOC, GPIOC, GPIO_PIN_7	
// PB0, PC8
#define TM2_CH2					RCU_GPIOB, GPIOB, GPIO_PIN_0
// PB1, PC9
//#define TM2_CH3					RCU_GPIOC, GPIOC, GPIO_PIN_9

#endif

#if	USE_TIMER_3

#define TM3_PRESCALER		10
#define TM3_FREQ				1000
#define TM3_PERIOD			SystemCoreClock / (TM3_PRESCALER * TM3_FREQ)

// PB6,PD12
#define TM3_CH0					RCU_GPIOB, GPIOB, GPIO_PIN_6
// PB7,PD13
//#define TM3_CH1					RCU_GPIOB, GPIOB, GPIO_PIN_7
// PB8,PD14
//#define TM3_CH2					RCU_GPIOB, GPIOB, GPIO_PIN_8
// PB9,PD15
//#define TM3_CH3					RCU_GPIOB, GPIOB, GPIO_PIN_9

#endif

#if	USE_TIMER_7

#define TM7_PRESCALER		1
#define TM7_PERIOD			SystemCoreClock / 10000

// PC6, PI5, PA5_ON, PA7_ON, PH13_ON
//#define TM7_CH0   			RCU_GPIOC, GPIOC, GPIO_PIN_6, GPIO_AF_3
#define TM7_CH0_ON			RCU_GPIOA, GPIOA, GPIO_PIN_7, GPIO_AF_3
// PC7, PI6, PB0_ON, PB14_ON, PH14_ON
//#define TM7_CH1   			RCU_GPIOC, GPIOC, GPIO_PIN_7,  GPIO_AF_3
#define TM7_CH1_ON			RCU_GPIOB, GPIOB, GPIO_PIN_14, GPIO_AF_3
// PC8, PI7, PB1_ON, PB15_ON, PH15_ON
//#define TM7_CH2					RCU_GPIOC, GPIOC, GPIO_PIN_8,  GPIO_AF_3
#define TM7_CH2_ON			RCU_GPIOB, GPIOB, GPIO_PIN_15, GPIO_AF_3
// PC9, PI2
//#define TM7_CH3					RCU_GPIOC, GPIOC, GPIO_PIN_9, GPIO_AF_3



#endif


#endif