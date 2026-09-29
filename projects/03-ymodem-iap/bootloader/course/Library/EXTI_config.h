#ifndef __EXTI_CONFIG_H__
#define __EXTI_CONFIG_H__

// 0表示禁用，1表示启用
#define USE_EXTI_0	0
#define USE_EXTI_1	0
#define USE_EXTI_2	0
#define USE_EXTI_3	0
#define USE_EXTI_4	0
#define USE_EXTI_5	0
#define USE_EXTI_6	0
#define USE_EXTI_7	0
#define USE_EXTI_8	0
#define USE_EXTI_9	0
#define USE_EXTI_10	0
#define USE_EXTI_11	0
#define USE_EXTI_12	0
#define USE_EXTI_13	0
#define USE_EXTI_14	0
#define USE_EXTI_15	0

#if USE_EXTI_0

/************************** EXTI0 **************************/
// GPIO
#define EXTI0_GPIO_RCU		RCU_GPIOA
#define EXTI0_GPIO_PORT		GPIOA
#define EXTI0_GPIO_PUPD		GPIO_PUPD_PULLUP
// EXTI
#define EXTI0_SYSCFG			EXTI_SOURCE_GPIOA
#define EXTI0_TRIG				EXTI_TRIG_RISING
// NVIC 
#define EXTI0_IRQ_PRIORITY	2, 2

#endif

#if USE_EXTI_1

/************************** EXTI1 **************************/
// GPIO & EXTI & NVIC
#define EXTI1_GPIO_RCU		  RCU_GPIOD
#define EXTI1_GPIO_PORT		  GPIOD
#define EXTI1_GPIO_PUPD		  GPIO_PUPD_PULLUP
#define EXTI1_SYSCFG			  EXTI_SOURCE_GPIOD
#define EXTI1_TRIG				  EXTI_TRIG_BOTH
#define EXTI1_IRQ_PRIORITY 	2, 2

#endif

#if USE_EXTI_3

/************************** EXTI3 **************************/
// GPIO & EXTI & NVIC
#define EXTI3_GPIO_RCU		  RCU_GPIOC
#define EXTI3_GPIO_PORT		  GPIOC
#define EXTI3_GPIO_PUPD		  GPIO_PUPD_PULLUP
#define EXTI3_SYSCFG			  EXTI_SOURCE_GPIOC
#define EXTI3_TRIG				  EXTI_TRIG_FALLING
#define EXTI3_IRQ_PRIORITY 	8, 0

#endif

#if USE_EXTI_5

#define EXTI5_SOFT	0		// 纯软实现
/************************** EXTI5 **************************/
#if !EXTI5_SOFT
// GPIO
#define EXTI5_GPIO_RCU		RCU_GPIOD
#define EXTI5_GPIO_PORT		GPIOD
#define EXTI5_GPIO_PUPD		GPIO_PUPD_PULLUP
// EXTI
#define EXTI5_SYSCFG			EXTI_SOURCE_GPIOD
#define EXTI5_TRIG				EXTI_TRIG_FALLING
#endif

// NVIC // EXTI5-9统一使用同一个中断
#define EXTI5_IRQ_PRIORITY	8, 0

#endif


#if USE_EXTI_6

/************************** EXTI6 **************************/
// GPIO & EXTI & NVIC
#define EXTI6_GPIO_RCU		RCU_GPIOD
#define EXTI6_GPIO_PORT		GPIOD
#define EXTI6_GPIO_PUPD		GPIO_PUPD_PULLUP
#define EXTI6_SYSCFG			EXTI_SOURCE_GPIOD
#define EXTI6_TRIG				EXTI_TRIG_BOTH
#define EXTI6_IRQ_PRIORITY 	2, 2

#endif

#if USE_EXTI_8

/************************** EXTI8 **************************/
// GPIO & EXTI & NVIC
#define EXTI8_GPIO_RCU		RCU_GPIOB
#define EXTI8_GPIO_PORT		GPIOB
#define EXTI8_GPIO_PUPD		GPIO_PUPD_PULLUP
#define EXTI8_SYSCFG			EXTI_SOURCE_GPIOB
#define EXTI8_TRIG				EXTI_TRIG_RISING
#define EXTI8_IRQ_PRIORITY 	2, 2

#endif

#if USE_EXTI_9

/************************** EXTI9 **************************/
// GPIO & EXTI & NVIC
#define EXTI9_GPIO_RCU		RCU_GPIOF
#define EXTI9_GPIO_PORT		GPIOF
#define EXTI9_GPIO_PUPD		GPIO_PUPD_PULLUP
#define EXTI9_SYSCFG			EXTI_SOURCE_GPIOF
#define EXTI9_TRIG				EXTI_TRIG_BOTH
#define EXTI9_IRQ_PRIORITY 	2, 2

#endif


#if USE_EXTI_13

/************************** EXTI13 **************************/
// GPIO & EXTI & NVIC
#define EXTI13_GPIO_RCU			RCU_GPIOC
#define EXTI13_GPIO_PORT		GPIOC
#define EXTI13_GPIO_PUPD		GPIO_PUPD_PULLUP
#define EXTI13_SYSCFG				EXTI_SOURCE_GPIOC
#define EXTI13_TRIG					EXTI_TRIG_BOTH
#define EXTI13_IRQ_PRIORITY 	2, 2

#endif
#endif