#include "bsp_buzzer.h"
#include "TIMER.h"
//			     C	 D    E 	F	 G	 A	  B	   C`
// static u16 hz[] = {523, 587, 659, 698, 784, 880, 988, 1047};

//			      C`	   D`     E`   F`	  G`	A`	  B`    C``
static u16 hz[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976, 2093};

// TIMER1_CH1

void Buzzer_init(){
  
}

// 根据音调在hz里的索引位置，取出对应的频率值
void Buzzer_beep(u16 hz_val_index){ // [1, 8]
	// 取出索引
	u16 hz_val = hz[hz_val_index - 1];
	Buzzer_play(hz_val);
}

void Buzzer_play(u16 hz_val){
  // 计算hz_val对应的周期
  uint32_t t_period = (SystemCoreClock / (hz_val * TM1_PRESCALER));
  
  // 更新周期
  TIMER_period_update(TIMER1, TM1_PRESCALER, t_period);
  
  // 重新启用
  timer_enable(TIMER1);
  
  // 设置占空比
  TIMER_channel_update(TIMER1, TIMER_CH_1, 50.0f);
}

void Buzzer_stop(){
  // 禁用Timer
  timer_disable(TIMER1);
}