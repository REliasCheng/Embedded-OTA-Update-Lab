#include "TIMER.h"

// GPIO 输出引脚---------------------------------
static void timer_gpio_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin,
                              uint32_t alt_func_num) {
  rcu_periph_clock_enable(rcu);
  gpio_mode_set(port, GPIO_MODE_AF, GPIO_PUPD_NONE, pin);
  gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pin);
  // af复用设置
  gpio_af_set(port, alt_func_num, pin);
}
/**
 * @brief Timer定时器初始化配置
 * 
 * @param rcu   TIMER的RCU
 * @param timer_periph  TIMERx(x=0,1,2,3,4,5,6,7,8,9,10,11,12,13)
 * @param t_perscaler   分频系数
 * @param t_period      周期
 */
static void timer_init_config(rcu_periph_enum rcu, uint32_t timer_periph,
                              uint16_t t_perscaler, uint32_t t_period) {
  /* 初始化TIMER的RCU */
  rcu_periph_clock_enable(rcu);
  /* deinit a TIMER */
  timer_deinit(timer_periph);

  timer_parameter_struct initpara;
  /* initialize TIMER counter */
  timer_struct_para_init(&initpara);

  /* 配置预分频系数, 可以实现更小的Timer频率。 */
  initpara.prescaler = t_perscaler - 1;
  /* 根据需要配置计数值 Max: 65535U */
  initpara.period = t_period - 1;
  /* initialize TIMER counter */
  timer_init(timer_periph, &initpara);
  /* enable a TIMER */
  timer_enable(timer_periph);
}

static void timer_channel_config(uint32_t timer_periph, uint16_t channel) {
  // CHANNEL输出通道配置    ---------------------------------
  timer_oc_parameter_struct ocpara;
  /* 初始化结构体参数 initialize TIMER channel output parameter struct */
  timer_channel_output_struct_para_init(&ocpara);
  /* 启用通道的正极(OP) PE9 */
  ocpara.outputstate = TIMER_CCX_ENABLE;
//   /* 启用通道的负极(ON) PE8 */
//   ocpara.outputnstate = TIMER_CCXN_ENABLE;

  /* 配置通道输出参数 configure TIMER channel output function */
  timer_channel_output_config(timer_periph, channel, &ocpara);
  /* 配置通道输出比较模式 configure TIMER channel output compare mode */
  timer_channel_output_mode_config(timer_periph, channel, TIMER_OC_MODE_PWM0);
}

/**
 * @brief TIMER 初始化
 * 1. 引脚初始化
 * 2. Timer初始化
 * 3. Timer中断初始化
 * 4. 打开互补输出（高级定时器）
 * 5. Timer启动
 */
void TIMER_init() {
  /* 升级所有Timer相关的时钟频率 */
  rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

#if USE_TIMER_1
  // Timer 初始化
  timer_init_config(RCU_TIMER1, TIMER1, TM1_PRESCALER, TM1_PERIOD);

#ifdef TM1_CH0
  // GPIO 初始化
  timer_gpio_config(TM1_CH0, GPIO_AF_1);
  // Timer 通道初始化
  timer_channel_config(TIMER1, TIMER_CH_0);
#endif

#ifdef TM1_CH1
  // GPIO 初始化
  timer_gpio_config(TM1_CH1, GPIO_AF_1);
  // Timer 通道初始化
  timer_channel_config(TIMER1, TIMER_CH_1);
#endif

  /* enable a TIMER */
  timer_enable(TIMER1);
#endif

#if USE_TIMER_2
  // Timer 初始化
  timer_init_config(RCU_TIMER2, TIMER2, TM2_PRESCALER, TM2_PERIOD);

#ifdef TM2_CH0
  // GPIO 初始化
  timer_gpio_config(TM2_CH0, GPIO_AF_2);
  // Timer 通道初始化
  timer_channel_config(TIMER2, TIMER_CH_0);
#endif

#ifdef TM2_CH1
  // GPIO 初始化
  timer_gpio_config(TM2_CH1, GPIO_AF_2);
  // Timer 通道初始化
  timer_channel_config(TIMER2, TIMER_CH_1);
#endif

#ifdef TM2_CH2
  // GPIO 初始化
  timer_gpio_config(TM2_CH2, GPIO_AF_2);
  // Timer 通道初始化
  timer_channel_config(TIMER2, TIMER_CH_2);
#endif

#endif

#if USE_TIMER_3
  // Timer 初始化
  timer_init_config(RCU_TIMER3, TIMER3, TM3_PRESCALER, TM3_PERIOD);

#ifdef TM3_CH0
  // GPIO 初始化
  timer_gpio_config(TM3_CH0, GPIO_AF_2);
  // Timer 通道初始化
  timer_channel_config(TIMER3, TIMER_CH_0);
#endif

#ifdef TM3_CH1
  // GPIO 初始化
  timer_gpio_config(TM3_CH1, GPIO_AF_2);
  // Timer 通道初始化
  timer_channel_config(TIMER3, TIMER_CH_1);
#endif

#ifdef TM3_CH2
  // GPIO 初始化
  timer_gpio_config(TM3_CH2);
  // Timer 通道初始化
  timer_channel_config(TIMER3, TIMER_CH_2);
#endif

#ifdef TM3_CH3
  // GPIO 初始化
  timer_gpio_config(TM3_CH3);
  // Timer 通道初始化
  timer_channel_config(TIMER3, TIMER_CH_3);
#endif

#endif

#if USE_TIMER_0 || USE_TIMER_7
  // 打开互补保护电路 -----------------------------
  // 只针对高级定时器TIMER0 & TIMER7， break事件保护电路
  timer_break_parameter_struct breakpara;
  /* 结构体初始化一些默认值 initialize TIMER break parameter struct */
  timer_break_struct_para_init(&breakpara);
  /* break事件输入的极性 */
  breakpara.breakpolarity = TIMER_BREAK_POLARITY_HIGH;
  /* 输出自动启用 */
  breakpara.outputautostate = TIMER_OUTAUTO_ENABLE;
  /* break事件输入功能启用 */
  breakpara.breakstate = TIMER_BREAK_ENABLE;
  /* 死区时间 */
  breakpara.deadtime = 10U;

#if USE_TIMER_0
  /* 配置TIMER的break保护功能 configure TIMER break function */
  timer_break_config(TIMER0, &breakpara);
  /* 启用 enable TIMER break function */
  timer_break_enable(TIMER0);
#endif
#if USE_TIMER_7
  /* 配置TIMER的break保护功能 configure TIMER break function */
  timer_break_config(TIMER7, &breakpara);
  /* 启用 enable TIMER break function */
  timer_break_enable(TIMER7);
#endif

#endif

}

/**
 * @brief 更新指定Timer和channel的占空比
 *
 * @param timer_periph  TIMERx(x=0,1,2,3,4,5,6,7,8,9,10,11,12,13)
 * @param channel   TIMER_CH_0, TIMER_CH_1, TIMER_CH_2, TIMER_CH_3
 * @param duty  占空比 [0, 100]
 */
void TIMER_channel_update(uint32_t timer_periph, uint16_t channel, float duty) {
    if (duty > 100) {
        duty = 100;
    } else if (duty < 0) {
        duty = 0;
    }

    // 根据duty计算pulse [0, 65535]
    uint32_t pulse = (uint32_t)(TIMER_CAR(timer_periph) * duty / 100);
    // 更新通道的占空比
    timer_channel_output_pulse_value_config(timer_periph, channel, pulse);
}

/**
 * @brief 更新指定Timer的周期
 *
 * @param timer_periph  TIMERx(x=0,1,2,3,4,5,6,7,8,9,10,11,12,13)
 * @param prescaler     分频系数 [1, 65536]
 * @param period        周期 [1, 65536]
 */
void TIMER_period_update(uint32_t timer_periph, uint16_t prescaler,
                         uint32_t period) {
    // 更新预分频系数
    timer_prescaler_config(timer_periph, prescaler - 1, TIMER_PSC_RELOAD_NOW);
    // 更新周期
    timer_autoreload_value_config(timer_periph, period - 1);
}