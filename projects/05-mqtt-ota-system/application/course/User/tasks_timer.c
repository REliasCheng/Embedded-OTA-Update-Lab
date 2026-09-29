#include "tasks_timer.h"
#include "tasks.h"

// Timer6 按照 1000hz 执行任务
volatile uint64_t task_tick = 0;

#define TIMER_RCU       RCU_TIMER6
#define TIMER           TIMER6
#define TIMER_PRESCALER 10
#define TIMER_PERIOD    (SystemCoreClock / TIMER_PRESCALER / 1000)

void tasks_timer_init(){

  // 初始化Timer6
  /* 升级频率*/
  rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
//  timer_init_config(RCU_TIMER3, TIMER3, TM3_PRESCALER, TM3_PERIOD); // 与通道无关

  rcu_periph_clock_enable(TIMER_RCU);
  
  timer_deinit(TIMER);
  /*初始化参数 */
  timer_parameter_struct initpara;
  /* initialize TIMER init parameter struct */
  timer_struct_para_init(&initpara);
  /* 根据需要配置值 分频系数 （可以实现更低的timer频率） */
  initpara.prescaler = TIMER_PRESCALER - 1;
  /* 1个周期的计数(period Max: 65535) Freq > 3662  */
  initpara.period		 = TIMER_PERIOD - 1;
  /* initialize TIMER counter */
  timer_init(TIMER, &initpara);
  /* enable a TIMER */
  timer_enable(TIMER);
  /* 配置中断优先级 */
  nvic_irq_enable(TIMER6_IRQn, 0, 1);
  /* 启用中断 */
  timer_interrupt_enable(TIMER, TIMER_INT_UP);
}

void TIMER6_IRQHandler(void){
  
  if(SET == timer_interrupt_flag_get(TIMER, TIMER_INT_FLAG_UP)){
    // 清除中断标记
    timer_interrupt_flag_clear(TIMER, TIMER_INT_FLAG_UP);
    
		task_tick++;
		
		// 1ms执行间隔（1000Hz）
		Task_switch_handler();
  }
}

uint64_t task_timer_get_tick(){
  return task_tick;
}