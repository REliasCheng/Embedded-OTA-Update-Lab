#include "wifi_drv.h"
#include "systick.h"
#include "USART.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static uint8_t * 	g_recvDataBuf;	// 数据缓存指针
static uint16_t 	g_pktLen;				// 数据长度
static bool				g_pktReceived;	// 是否收到数据


void USART2_on_recv(uint8_t* data, uint32_t len){
	g_recvDataBuf = data;
	g_pktLen = len;
	g_pktReceived = true;
}

// 接收数据
uint16_t RecvWifiModuleStr(char* buffer){
	if(g_pktReceived){	// 收到数据了
		g_pktReceived = false;
		// 把数据拷贝给外边的buffer
		memcpy(buffer, g_recvDataBuf, g_pktLen);
		printf("USART2 recv->[%d]: %s\n", g_pktLen, buffer);
		// 清理数据,全部清0
		memset(g_recvDataBuf, 0, WIFI_MAX_BUF_SIZE);
		return g_pktLen;
	}
	return 0;
}

char g_recvStrBuf[WIFI_MAX_BUF_SIZE];
uint16_t g_recvStrLen = 0;
/**********************************************************
 * @brief AT指令处理器
 * @param cmd 指令内容
 * @param rsp 期待的正确响应
 * @param timeoutMs 超时时间
 * @return 
 **********************************************************/
WifiCommState_t AtCmdHandle(char* cmd, char* rsp, uint32_t timeoutMs){
	static WifiCommState_t s_commState = WIFI_COMM_OK;
	static uint64_t s_sendCmdTime;
	
	// 首次进来，发送，更新状态
	if(s_commState != WIFI_COMM_WAIT){
	
		// 发AT指令
		if(cmd != NULL){
			SendWifiModuleStr(cmd);
		}
		// 记录开始发送的时间
		s_sendCmdTime = systick_get_tick();
		// 更新状态
		s_commState = WIFI_COMM_WAIT;
	}
	
	// Waiting...
	// 超时检测
	if((systick_get_tick() - s_sendCmdTime) > timeoutMs){
		s_commState = WIFI_COMM_FAIL;
		return s_commState;
	}
	
	// 成功检测
	g_recvStrLen = RecvWifiModuleStr(g_recvStrBuf);
	if(g_recvStrLen > 0 && strstr(g_recvStrBuf, rsp) != NULL){
		s_commState = WIFI_COMM_OK;
		return s_commState;
	}
	
	return s_commState;
}
// 清理Wifi字符串
void ClearRecvWifiStr(void)
{
  memset(g_recvStrBuf, 0, WIFI_MAX_BUF_SIZE);
	g_recvStrLen = 0;
}

static void WifiGpioInit(rcu_periph_enum rcu, uint32_t port, uint32_t pin) {
  // 1. 时钟初始化
  rcu_periph_clock_enable(rcu);
  // 2. 配置GPIO 输入输出模式
  gpio_mode_set(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
  // 3. 配置GPIO 输出选项
  gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pin);
  // 4. 默认输出电平
  gpio_bit_write(port, pin, RESET);
}

#define RESET_GPIO	GPIOB, GPIO_PIN_12

void WifiModuleDrvInit(void){
	// 初始化GPIO
	WifiGpioInit(RCU_GPIOB, RESET_GPIO);
	
	// 初始化USART...	
	
	// 硬件重置
	HwResetWifiModule();
}

// 发送数据
void SendWifiModuleStr(char *sendStr){
	printf("usart2 send: %s\n", sendStr);
	USART2_send_string(sendStr);
}

// 硬件重置
void HwResetWifiModule(void){
	printf("Wifi module hardware reset!\n");
	gpio_bit_reset(RESET_GPIO);
	delay_1ms(100);
	gpio_bit_set(RESET_GPIO);
}