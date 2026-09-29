#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "gd32f4xx.h"

#define FLASH_SIZE                        0x80000			// 512K

// App的开始位置
#define APP_ADDR_IN_FLASH                 0x8004000

#define FLASH_APP_SIZE                    (FLASH_SIZE - (APP_ADDR_IN_FLASH - 0x08000000))  //计算app空间可用大小


void UpdateApp(void);

#endif