#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include <string.h>
#include "USART.h"
#include "I2C.h"
#include "delay.h"
#include "flash_drv.h"
#include "../App/App.h"

#include "iap_driver.h"
#include "tasks.h"
#include "tasks_timer.h"
#include "store_app.h"
#include "update.h"
#include "wifi_drv.h"

/***********************
任务：模板工程
fromelf --bin --output .\Objects\GD32F407.bin .\Objects\GD32F407.axf

************************/
void USART0_on_recv(uint8_t* data, uint32_t len) {
  printf("recv[%d]:%s\n", len, data);
	
}

#define RAM_START_ADDRESS    0x20000000
#define RAM_SIZE             0x20000			// 128K 

typedef void (*pFunction)(void);

static void BootToApp(void)
{
	uint32_t stackTopAddr = *(volatile uint32_t*)APP_ADDR_IN_FLASH; 
	
	printf("addr: 0x%08X\n", stackTopAddr);
	if (stackTopAddr > RAM_START_ADDRESS && stackTopAddr < (RAM_START_ADDRESS + RAM_SIZE)) //判断栈顶地址是否在合法范围内
	{
		NVIC_DisableIRQ(USART0_IRQn);
		NVIC_DisableIRQ(USART2_IRQn);
		NVIC_DisableIRQ(SysTick_IRQn);
		NVIC_DisableIRQ(TIMER6_IRQn);
		
		__disable_irq();
		__set_MSP(stackTopAddr);
		uint32_t resetHandlerAddr = *(volatile uint32_t*) (APP_ADDR_IN_FLASH + 4);
		/* Jump to user application */
		pFunction Jump_To_Application = (pFunction) resetHandlerAddr; // int *p = (int *)0x8003145
		/* Initialize user application's Stack Pointer */
		Jump_To_Application();
	}else {
		printf("addr: 不在合法范围\n");
	}
	NVIC_SystemReset();
}

int main(void) {
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
	
	// 初始化Timer定时器
	tasks_timer_init();
	
	// 初始化所有的任务
//	Task_init();
	App_OTA_init();
	WifiModuleDrvInit();
	
	printf("Bootloader init!\n");
	
	// 检查是否需要升级
	bool isNeedUpdate = CheckNeedUpdate();
	
  while(1) {		
		// 显示OLED
		App_OTA_task();
		
		// 检查并更新数据
		if(isNeedUpdate){
//			printf("Need Update\n");
			uint8_t success = UpdateApp();
			if(success == 1){
				// 成功，更新标记
				isNeedUpdate = CheckNeedUpdate();
			}else if(success == 2){
				printf("升级失败，未发现新固件，重启至App!\n");
				isNeedUpdate = false;
			}
			
		}else {
			BootToApp();
		}
		
  }
}
