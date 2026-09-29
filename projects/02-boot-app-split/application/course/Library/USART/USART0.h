#ifndef __USART0_H__
#define __USART0_H__

#include "gd32f4xx.h"
#include "USART_config.h"

void USART0_init();

// 发送1字节数据
void USART0_send_byte(uint8_t data);

// 发送字节数组
void USART0_send_data(uint8_t* data, uint32_t len);

// 发送字符串
void USART0_send_string(char *data);

#if USART0_RECV_CALLBACK
extern void USART0_on_recv(uint8_t* data, uint32_t len);
#endif

#if USART0_RECV_BYTE_CALLBACK
extern void USART0_on_byte_recv(uint8_t data);
#endif


#endif