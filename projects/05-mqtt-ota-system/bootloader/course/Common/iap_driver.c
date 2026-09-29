#include "gd32f4xx.h"
#include "store_app.h"

void iap_init_irq(void) {
  // 设置中断向量表起始地址偏移量
  nvic_vector_table_set(NVIC_VECTTAB_FLASH, BOOTLOADER_SIZE);
  // 启用中断
  __enable_irq();
}

void iap_reset_to_boot(void) {
	// 关闭所有中断
  __disable_irq();
  // 系统复位函数
  NVIC_SystemReset();
}