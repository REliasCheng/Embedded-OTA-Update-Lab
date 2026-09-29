#ifndef __USART_CONFIG_H__
#define __USART_CONFIG_H__

#define USE_USART0	1
#define USE_USART1	0
#define USE_USART2	1
#define USE_UART3		0
#define USE_UART4		0
#define USE_USART5	0
#define USE_UART6		0
#define USE_UART7		0

/********************************************* USART0 ************************************/
#if USE_USART0

// 通过宏定义，初始化一些变量值
// 是否开启接收
#define USART0_RECV_CALLBACK			1
#define USART0_RECV_BYTE_CALLBACK 0
#define USART0_PRINTF							1

// GPIO ----------------------------------------------------------------
// TX
#define USART0_TX_RCU 	 	RCU_GPIOA
#define USART0_TX_PORT  	GPIOA
#define USART0_TX_PIN   	GPIO_PIN_9

// RX
#define USART0_RX_RCU 		RCU_GPIOA
#define USART0_RX_PORT  	GPIOA
#define USART0_RX_PIN   	GPIO_PIN_10

#define USART0_AF  	 			GPIO_AF_7

// USART ----------------------------------------------------------------
// USART 波特率
#define USART0_BAUDRATE 	 					115200
#define	USART0_RX_BUFFER_LEN				1024
#define USART0_IRQ_PRIORIY					2, 2

// DMA ------------------------------------------------------------------
#define USART0_TX_DMA_ENABLE	0
#define USART0_RX_DMA_ENABLE	0

#if USART0_TX_DMA_ENABLE
#define USART0_TX_DMA_RCU						RCU_DMA1
#define USART0_TX_DMA_PERIPH_CH			DMA1, DMA_CH7
#define USART0_TX_DMA_CH_SUB				DMA_SUBPERI4
#endif

#if USART0_RX_DMA_ENABLE
#define USART0_RX_DMA_RCU						RCU_DMA1
#define USART0_RX_DMA_PERIPH_CH			DMA1, DMA_CH2
#define USART0_RX_DMA_CH_SUB				DMA_SUBPERI4
#endif


#endif

/********************************************* USART1 ************************************/
#if USE_USART1


// 通过宏定义，初始化一些变量值
// 是否开启接收
#define USART1_RECV_CALLBACK			0
#define USART1_RECV_BYTE_CALLBACK 0
#define USART1_PRINTF							0

// GPIO ----------------------------
// TX, PA2,PD5
#define USART1_TX_RCU 	 	RCU_GPIOD
#define USART1_TX_PORT  	GPIOD
#define USART1_TX_PIN   	GPIO_PIN_5

// RX, PA3,PD6
#define USART1_RX_RCU 		RCU_GPIOD
#define USART1_RX_PORT  	GPIOD
#define USART1_RX_PIN   	GPIO_PIN_6

#define USART1_AF  	 								GPIO_AF_7
// USART ---------------------------
// USART 波特率
#define USART1_BAUDRATE 	 					9600
#define	USART1_RX_BUFFER_LEN				1024
#define USART1_IRQ_PRIORIY					6, 0

// DMA -----------------------------
#define USART1_TX_DMA_ENABLE	0
#define USART1_RX_DMA_ENABLE	0

#if USART1_TX_DMA_ENABLE
#define USART1_TX_DMA_RCU						RCU_DMA0
#define USART1_TX_DMA_PERIPH_CH			DMA0, DMA_CH6
#define USART1_TX_DMA_CH_SUB				DMA_SUBPERI4
#endif

#if USART1_RX_DMA_ENABLE
#define USART1_RX_DMA_RCU						RCU_DMA0
#define USART1_RX_DMA_PERIPH_CH			DMA0, DMA_CH5
#define USART1_RX_DMA_CH_SUB				DMA_SUBPERI4
#endif


#endif


/********************************************* USART2 ************************************/
#if USE_USART2


// 通过宏定义，初始化一些变量值
// 是否开启接收
#define USART2_RECV_CALLBACK				1
#define USART2_RECV_BYTE_CALLBACK 	0
#define USART2_PRINTF								0

// GPIO ----------------------------
// TX, PB10,PC10,PD8	
#define USART2_TX_RCU 	 						RCU_GPIOB
#define USART2_TX_PORT  						GPIOB
#define USART2_TX_PIN   						GPIO_PIN_10
					
// RX, PB11,PC5,PD9					
#define USART2_RX_RCU 							RCU_GPIOB
#define USART2_RX_PORT  						GPIOB
#define USART2_RX_PIN   						GPIO_PIN_11

#define USART2_AF  	 								GPIO_AF_7
// USART ---------------------------
// USART 波特率
#define USART2_BAUDRATE 	 					115200
#define	USART2_RX_BUFFER_LEN				1024
#define USART2_IRQ_PRIORIY					3, 0

// DMA -----------------------------
#define USART2_TX_DMA_ENABLE	0
#define USART2_RX_DMA_ENABLE	1

#if USART2_TX_DMA_ENABLE
#define USART2_TX_DMA_RCU						RCU_DMA0
#define USART2_TX_DMA_PERIPH_CH			DMA0, DMA_CH3
#define USART2_TX_DMA_CH_SUB				DMA_SUBPERI4
#endif

#if USART2_RX_DMA_ENABLE
#define USART2_RX_DMA_RCU						RCU_DMA0
#define USART2_RX_DMA_PERIPH_CH			DMA0, DMA_CH1
#define USART2_RX_DMA_CH_SUB				DMA_SUBPERI4
#endif


#endif

#endif