#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include <string.h>
#include "USART.h"
#include "I2C.h"
#include "delay.h"
#include "flash_drv.h"

#include "iap_driver.h"
#include "tasks.h"
#include "tasks_timer.h"

#define APP_ADDR_IN_FLASH					0x08008000

/***********************
任务：模板工程
fromelf --bin --output .\Objects\GD32F407.bin .\Objects\GD32F407.axf

************************/
void USART0_on_recv(uint8_t* data, uint32_t len) {
  printf("recv[%d]:%s\n", len, data);
	
	if(data[0] == 'U'){
		iap_reset_to_boot();
	}
}

static void GPIO_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin) {
  // 1. 时钟初始化
  rcu_periph_clock_enable(rcu);
  // 2. 配置GPIO 输入输出模式
  gpio_mode_set(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
  // 3. 配置GPIO 输出选项
  gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pin);
  // 4. 默认输出电平
  gpio_bit_write(port, pin, RESET);
}

int main(void) {
	// 初始化中断向量表的偏移量 0x4000 (16K)
	iap_init_irq();
	
	int i = 0;
	
  // 配置全局中断分组
  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
  // 初始化系统嘀嗒定时器
  systick_config();
	// 系统睡眠工具
	DelayInit();
  // 初始化USART
  USART_init();
	// 初始化I2C
	I2C_init();
	// GPIO
	GPIO_config(RCU_GPIOB, GPIOB, GPIO_PIN_2);
	
	// 初始化所有的任务
	Task_init();
	
	// 初始化Timer定时器
	tasks_timer_init();
	
	printf("App v1.0 init!\n");
  while(1) {
		Task_exec_handler();
  }
}
