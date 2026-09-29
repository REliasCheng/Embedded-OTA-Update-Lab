#ifndef __WIFI_DRV_H__
#define __WIFI_DRV_H__

#include "gd32f4xx.h"

#define WIFI_MAX_BUF_SIZE           1024

extern char g_recvStrBuf[];
extern uint16_t g_recvStrLen;

typedef enum {

	WIFI_COMM_WAIT = 0, // 通讯等待中
	WIFI_COMM_OK,		// 验证成功
	WIFI_COMM_FAIL,	// 验证失败 ( 响应与预期不符， 超时 )
	
} WifiCommState_t;

WifiCommState_t AtCmdHandle(char* cmd, char* rsp, uint32_t timeoutMs);
/*******************
1. 接收数据的外设（串口，蓝牙, I2C...）
2. 硬件重置的IO引脚
********************/
void WifiModuleDrvInit(void);	// 大驼峰命名法

// 接收数据
uint16_t RecvWifiModuleStr(char* buffer);

//清理Wifi字符串
void ClearRecvWifiStr(void);
	
// 发送数据
void SendWifiModuleStr(char *sendStr);

// 硬件重置
void HwResetWifiModule(void);

#endif