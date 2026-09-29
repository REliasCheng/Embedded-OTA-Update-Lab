#include "bsp_keys.h"


static void GPIO_config(void) {

  /********************* PC0 按键引脚 *********************/
  // 时钟初始化
  rcu_periph_clock_enable(RCU_GPIOC);
  // 配置GPIO模式
  gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3);
}


void Keys_init() {
  GPIO_config();
}

FlagStatus pre_state0 = SET;// 默认高电平抬起
FlagStatus pre_state1 = SET;// 默认高电平抬起
FlagStatus pre_state2 = SET;// 默认高电平抬起
FlagStatus pre_state3 = SET;// 默认高电平抬起
void Keys_scan() {
// PC0
  FlagStatus state0 = gpio_input_bit_get(GPIOC, GPIO_PIN_0);
  if (state0 != pre_state0) {
    if(state0 == RESET) {  // 当前低电平, 上一次为高电平，按下
      Keys_on_keydown(0);
    }else{
      Keys_on_keyup(0);
    }
    
    pre_state0 = state0;
  }
  // PC1
  FlagStatus state1 = gpio_input_bit_get(GPIOC, GPIO_PIN_1);
  if (state1 != pre_state1) {
    if(state1 == RESET) {  // 当前低电平, 上一次为高电平，按下
      Keys_on_keydown(1);
    }else{
      Keys_on_keyup(1);
    }
    pre_state1 = state1;
  }
  // PC2
  FlagStatus state2 = gpio_input_bit_get(GPIOC, GPIO_PIN_2);
  if (state2 != pre_state2) {
    if(state2 == RESET) {  // 当前低电平, 上一次为高电平，按下
      Keys_on_keydown(2);
    } else {
      Keys_on_keyup(2);
    
    }
    pre_state2 = state2;
  }
  // PC3
  FlagStatus state3 = gpio_input_bit_get(GPIOC, GPIO_PIN_3);
  if (state3 != pre_state3) {
    if(state3 == RESET) {  // 当前低电平, 上一次为高电平，按下
      Keys_on_keydown(3);
    }else {
      Keys_on_keyup(3);
    }
    pre_state3 = state3;
  }
}