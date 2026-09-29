#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

uint8_t UpdateApp(void);
// 获取最新版本
bool GetNewVersionInfo(char *version, uint32_t *binSize);
// 获取当前进度 [0,100]
uint8_t GetUpdateProgress(void);
#endif