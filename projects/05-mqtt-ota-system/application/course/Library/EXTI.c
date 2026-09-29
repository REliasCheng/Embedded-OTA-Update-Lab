#include "EXTI.h"



//#define NVIC_EXTI_IRQHandler	EXTI5_9_IRQHandler

static void EXTI_config(
  rcu_periph_enum gpio_rcu_periph, uint32_t gpio_port, uint32_t pull_up_down, uint32_t gpio_pin,
  uint8_t exti_port, uint8_t exti_pin, exti_line_enum linex, exti_trig_type_enum trig_type,
  uint8_t nvic_irq, uint8_t nvic_irq_pre_priority, uint8_t nvic_irq_sub_priority

) {

  // 初始化PA0外部中断  EXTI不是复用
  // GPIO -------------------------------------------------
  rcu_periph_clock_enable(gpio_rcu_periph);
  // gpio
  gpio_mode_set(gpio_port, GPIO_MODE_INPUT, pull_up_down, gpio_pin);

  // EXTI-------------------------------------------------
  rcu_periph_clock_enable(RCU_SYSCFG);
  // 将key EXTI line连接到key GPIO pin上
  syscfg_exti_line_config(exti_port, exti_pin);
  // 启用EXTI0
  exti_init(linex, EXTI_INTERRUPT, trig_type);

  // NVIC -------------------------------------------------
  // 启用并配置中断优先级
  nvic_irq_enable(nvic_irq, nvic_irq_pre_priority, nvic_irq_sub_priority);
  // 启用外部中断 EXTI0
  exti_interrupt_enable(linex);
  // 可选操作，清理之前中断标记
  exti_interrupt_flag_clear(linex);
}

static void EXTI_config_soft(
  exti_line_enum linex,
  uint8_t nvic_irq, uint8_t nvic_irq_pre_priority, uint8_t nvic_irq_sub_priority

) {

  // EXTI-------------------------------------------------
  rcu_periph_clock_enable(RCU_SYSCFG);
  // 启用EXTI
  exti_init(linex, EXTI_INTERRUPT, EXTI_TRIG_NONE);

  // NVIC -------------------------------------------------
  // 启用并配置中断优先级
  nvic_irq_enable(nvic_irq, nvic_irq_pre_priority, nvic_irq_sub_priority);
  // 启用外部中断 EXTI0
  exti_interrupt_enable(linex);
  // 可选操作，清理之前中断标记
  exti_interrupt_flag_clear(linex);
}




void EXTI_init() {

#if USE_EXTI_0
  // EXTI_0
  EXTI_config(
    EXTI0_GPIO_RCU, EXTI0_GPIO_PORT, EXTI0_GPIO_PUPD, GPIO_PIN_0, // GPIO
    EXTI0_SYSCFG, EXTI_SOURCE_PIN0, EXTI_0, EXTI0_TRIG,		// EXTI
    EXTI0_IRQn, EXTI0_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_1
  // EXTI_1
  EXTI_config(
    EXTI1_GPIO_RCU, EXTI1_GPIO_PORT, EXTI1_GPIO_PUPD, GPIO_PIN_1, // GPIO
    EXTI1_SYSCFG, EXTI_SOURCE_PIN1, EXTI_1, EXTI1_TRIG,		// EXTI
    EXTI1_IRQn, EXTI1_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_3
  // EXTI_3
  EXTI_config(
    EXTI3_GPIO_RCU, EXTI3_GPIO_PORT, EXTI3_GPIO_PUPD, GPIO_PIN_3, // GPIO
    EXTI3_SYSCFG, EXTI_SOURCE_PIN3, EXTI_3, EXTI3_TRIG,		// EXTI
    EXTI3_IRQn, EXTI3_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_5

#if !EXTI5_SOFT

  // EXTI_5
  EXTI_config(
    EXTI5_GPIO_RCU, EXTI5_GPIO_PORT, EXTI5_GPIO_PUPD, GPIO_PIN_5, // GPIO
    EXTI5_SYSCFG, EXTI_SOURCE_PIN5, EXTI_5, EXTI5_TRIG,		// EXTI
    EXTI5_9_IRQn, EXTI5_IRQ_PRIORITY
  );
#else

  EXTI_config_soft(
    EXTI_5,
    EXTI5_9_IRQn, EXTI5_IRQ_PRIORITY
  );

#endif

#endif

#if USE_EXTI_6
  // EXTI_6
  EXTI_config(
    EXTI6_GPIO_RCU, EXTI6_GPIO_PORT, EXTI6_GPIO_PUPD, GPIO_PIN_6, // GPIO
    EXTI6_SYSCFG, EXTI_SOURCE_PIN6, EXTI_6, EXTI6_TRIG,		// EXTI
    EXTI5_9_IRQn, EXTI6_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_8
  // EXTI_8
  EXTI_config(
    EXTI8_GPIO_RCU,
    EXTI8_GPIO_PORT,
    EXTI8_GPIO_PUPD,
    GPIO_PIN_8, // GPIO
    EXTI8_SYSCFG, EXTI_SOURCE_PIN8, EXTI_8, EXTI8_TRIG,		// EXTI
    EXTI5_9_IRQn, EXTI8_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_9
  // EXTI_9
  EXTI_config(
    EXTI9_GPIO_RCU,
    EXTI9_GPIO_PORT,
    EXTI9_GPIO_PUPD,
    GPIO_PIN_9, // GPIO
    EXTI9_SYSCFG, EXTI_SOURCE_PIN9, EXTI_9, EXTI9_TRIG,		// EXTI
    EXTI5_9_IRQn, EXTI9_IRQ_PRIORITY
  );
#endif

#if USE_EXTI_13
  // EXTI_13
  EXTI_config(
    EXTI13_GPIO_RCU,
    EXTI13_GPIO_PORT,
    EXTI13_GPIO_PUPD,
    GPIO_PIN_13, // GPIO
    EXTI13_SYSCFG, EXTI_SOURCE_PIN13, EXTI_13, EXTI13_TRIG,		// EXTI
    EXTI10_15_IRQn, EXTI13_IRQ_PRIORITY
  );
#endif

}

void EXTI_soft_trig(exti_line_enum linex) {

  exti_software_interrupt_enable(linex);
}

//#define INTERRUPT_FLAG_TRIG(exti_line_enum linex)		              \
//  if (SET == exti_interrupt_flag_get(linex)) {     \
//		/* 清理标记，避免重复进入中断函数 */              \
//		exti_interrupt_flag_clear(linex);              \
//		EXTI_on_trig(linex);                           \
//  }																									\

#if USE_EXTI_0
void EXTI0_IRQHandler(void) {
  if (SET == exti_interrupt_flag_get(EXTI_0)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_0);
    EXTI0_on_trig();
  }
}
#endif

#if USE_EXTI_1
void EXTI1_IRQHandler(void) {
  if (SET == exti_interrupt_flag_get(EXTI_1)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_1);
    EXTI1_on_trig();
  }
}
#endif
#if USE_EXTI_2
void EXTI2_IRQHandler(void) {
  if (SET == exti_interrupt_flag_get(EXTI_2)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_2);
    EXTI2_on_trig();
  }
}
#endif
#if USE_EXTI_3
void EXTI3_IRQHandler(void) {
  if (SET == exti_interrupt_flag_get(EXTI_3)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_3);
    EXTI3_on_trig();
  }
}
#endif

#if USE_EXTI_5 || USE_EXTI_6 || USE_EXTI_7 || USE_EXTI_8 || USE_EXTI_9
void EXTI5_9_IRQHandler(void) {

#if USE_EXTI_5
  if (SET == exti_interrupt_flag_get(EXTI_5)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_5);
    EXTI5_on_trig();
  }
#endif

#if USE_EXTI_6
  if (SET == exti_interrupt_flag_get(EXTI_6)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_6);
    EXTI6_on_trig();
  }
#endif

#if USE_EXTI_7
  if (SET == exti_interrupt_flag_get(EXTI_7)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_7);
    EXTI7_on_trig();
  }
#endif

#if USE_EXTI_8
  if (SET == exti_interrupt_flag_get(EXTI_8)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_8);
    EXTI8_on_trig();
  }
#endif

#if USE_EXTI_9

  if (SET == exti_interrupt_flag_get(EXTI_9)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_9);
    EXTI9_on_trig();
  }
#endif

}
#endif

#if USE_EXTI_10 || USE_EXTI_11 || USE_EXTI_12 || USE_EXTI_13 || USE_EXTI_14|| USE_EXTI_15
void EXTI10_15_IRQHandler(void) {

#if USE_EXTI_13
  if (SET == exti_interrupt_flag_get(EXTI_13)) {
    /* 清理标记，避免重复进入中断函数 */
    exti_interrupt_flag_clear(EXTI_13);
    EXTI13_on_trig();
  }
#endif

}
#endif