#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include <string.h>
#include "USART.h"

#include "flash_drv.h"

/***********************
任务：模板工程
fromelf --bin --output .\Objects\GD32F407.bin .\Objects\GD32F407.axf

************************/
void USART0_on_recv(uint8_t* data, uint32_t len) {
  printf("recv[%d]:%s\n", len, data);
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

#define APP_ADDR_IN_FLASH					0x08004000

#define RAM_START_ADDRESS					0x20000000
#define RAM_SIZE									0x20000				// 128KB

typedef void (*pFunction)(void);

static void BootToApp(void){
	// 加载存放在0x0800_4000的App固件，取出栈顶地址（0x20001418）
	uint32_t stackTopAddr = *((__IO uint32_t *)APP_ADDR_IN_FLASH);
	
	printf("stackTopAddr: 0x%08X\n", stackTopAddr);
	
	// 栈顶地址在正确的范围 [0x2000_0000, 0x20020000)
	if(stackTopAddr >= RAM_START_ADDRESS && stackTopAddr < (RAM_START_ADDRESS + RAM_SIZE)){
	
		// 1. 禁用所有的外部中断
		__disable_irq();
		
		// 2. 设置栈顶地址
		__set_MSP(stackTopAddr); // 0xFFFFFFFF
		
		// 3. 加载存放在0x0800_4004的Reset_Handler，跳转到用户的App （执行Reset_Handler） 0x0800019d 
		uint32_t resetHandlerAddr = *((__IO uint32_t *)(APP_ADDR_IN_FLASH + 4));
		
		printf("resetHandlerAddr: 0x%08X\n", resetHandlerAddr);
		
		pFunction Jump_To_Application = (pFunction) resetHandlerAddr;	// uint32_t *p = (uint32_t *)0x0800019d
		// 执行函数
		Jump_To_Application();
	}else {
		printf("栈顶地址不在合法范围，无法启动\n");
	}
	
	// 重启MCU
	NVIC_SystemReset();
	
}

#define BOOT_DELAY_COUNT		8000  // 8s

#define DOWNLOAD_KEY_VALUE                 '1' // 0x31
#define EXECUTE_KEY_VALUE                  '2' // 0x32

bool GetKeyPressed(uint8_t *key){
	return USART0_ReceiveByte(key);
}

void UpdateApp(){

}

// 显示菜单，等待接收指令
static void main_menu_cmd(){

	printf("按下任意按键，停止自启动：\n");
	
	uint8_t serialKey;
	
	// 获取当前开机时间戳
	uint64_t timCount = systick_get_tick();
	uint8_t bootDelayNow = 0;
	uint8_t bootDelayLast = 0;
	
	// 循环等待8000ms，循环结束条件：
	// 1. 直到时间结束
	// 2. 用户按下了任意按键(发送了任意字节过来)
	while(timCount < BOOT_DELAY_COUNT && !USART0_ReceiveByte(&serialKey)){
		// 获取当前系统启动时间戳
		timCount = systick_get_tick();
		
		// 将毫秒值转成秒值，进而转成倒计时
		bootDelayNow = (BOOT_DELAY_COUNT - timCount) / 1000; // 8, 7, 6, 
		
		// 如果秒钟发生了变化
		if(bootDelayNow != bootDelayLast){
		
			printf("%2d\n", bootDelayNow);
			bootDelayLast = bootDelayNow;
		}
	}
	
	if(timCount >= BOOT_DELAY_COUNT){
		// 没有收到任何消息，8s过去了
		printf("Boot\n");
		BootToApp();
	}
	
	// 如果循环是因为按键结束的，timCount一定小于BOOT_DELAY_COUNT
	while(1){
    printf("\r\n\n========================= 主菜单 =============================\r\n\n");
    printf("************【1】下载固件到内部Flash*******************\r\n\n");
    printf("************【2】启动 App******************************\r\n\n");
    printf("\r\n==============================================================\r\n\n");
		
		// 阻塞接收用户进一步的输入
		while(!USART0_ReceiveByte(&serialKey));
		
		printf("key: 0x%02X -> %c\n", serialKey, serialKey);

		if(serialKey == DOWNLOAD_KEY_VALUE){
			printf("Download!\n");
			UpdateApp();
		}
		
		if(serialKey == EXECUTE_KEY_VALUE){
			printf("Boot to App!\n");
			BootToApp();
		}
		
	}
	
}

int main(void) {
	int i = 0;
	
  // 配置全局中断分组
  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
  // 初始化系统嘀嗒定时器
  systick_config();
  // 初始化USART
  USART_init();
	// GPIO
	GPIO_config(RCU_GPIOB, GPIOB, GPIO_PIN_2);
	
	printf("Bootloader init111!\n");
	
//	FlashDrvTest();
	// 加载App
	
  while(1) {
//		printf("led: %d\n", i);
//		delay_1ms(200);
		
		main_menu_cmd();
		
  }
}
