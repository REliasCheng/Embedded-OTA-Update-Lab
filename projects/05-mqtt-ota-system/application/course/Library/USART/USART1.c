
#include "USART1.h"
#include <string.h>

#if USART1_PRINTF

#include <stdio.h>
// printf 发送
int fputc(int ch, FILE *f) {
  USART1_send_byte((uint8_t) ch);
  return ch;
}

#endif

// static 只有当前文件可见以下两个变量
static uint8_t 		rx_buffer[USART1_RX_BUFFER_LEN];
static uint32_t 	rx_cnt = 0;

// DMA用到的USART数据收发寄存器
#define USART1_DATA_ADDR						((uint32_t)&USART_DATA(USART1))

#if USART1_TX_DMA_ENABLE
static void USART1_TX_DMA_init() {

  // TX数据发送， 内存 -> USART1外设
  // SRC数据源：地址待定，长度待定
  // DST目标	：USART_DATA_ADDR

  // RCU -------------------------------------
  rcu_periph_clock_enable(USART1_TX_DMA_RCU);

  // DMA -------------------------------------
  // 重置DMA_ USART1_TX
  dma_deinit(USART1_TX_DMA_PERIPH_CH);
  dma_single_data_parameter_struct init_struct;
  /* initialize the DMA single data mode parameters struct with the default values */
  dma_single_data_para_struct_init(&init_struct);
  // 拷贝方向 (内存到外设)
  init_struct.direction           = DMA_MEMORY_TO_PERIPH;

  // SRC源头 内存 				  -------
//	init_struct.memory0_addr        = (uint32_t)dst;
  init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
  // 拷贝数据数量
//	init_struct.number              = sizeof(src) / sizeof(src[0]);
  // 每个数据的大小(BIT数量)
  init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;

  // DST目标 外设	periph	 -------
//	uint32_t a = USART_DATA(USART1);
  init_struct.periph_addr         = USART1_DATA_ADDR;
  init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;

  /* 初始化 DMA single data mode initialize */
  dma_single_data_mode_init(USART1_TX_DMA_PERIPH_CH, &init_struct);

  /* 选择指定通道子集 DMA channel peripheral select** */
  dma_channel_subperipheral_select(USART1_TX_DMA_PERIPH_CH, USART1_TX_DMA_CH_SUB);

  // NVIC ------------------------------------传输完成中断（可选）
//	nvic_irq_enable(DMA1_Channel7_IRQn, 2, 2);
  /* enable DMA interrupt */
//	dma_interrupt_enable(DMA_PERIPH_CH, DMA_CHXCTL_FTFIE);
}
#endif

#if USART1_RX_DMA_ENABLE
static void USART1_RX_DMA_init() {

  // RX数据接收：USART1外设 -> 内存
  // SRC数据源： USART1数据寄存器USART1_DATA_ADDR
  // DST目标	： rx_buffer

  // RCU -------------------------------------
  rcu_periph_clock_enable(USART1_RX_DMA_RCU);

  // DMA -------------------------------------
  // 重置DMA_ USART1_TX
  dma_deinit(USART1_RX_DMA_PERIPH_CH);
  dma_single_data_parameter_struct init_struct;
  /* initialize the DMA single data mode parameters struct with the default values */
  dma_single_data_para_struct_init(&init_struct);
  // 拷贝方向 (内存到外设)
  init_struct.direction           = DMA_PERIPH_TO_MEMORY;

  // SRC源头 外设	periph	 -------
  init_struct.periph_addr         = USART1_DATA_ADDR;
  init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
  // 拷贝数据数量
  init_struct.number            	= USART1_RX_BUFFER_LEN;
  // 每个数据的大小(BIT数量)
  init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;

  // DST目标 内存 				  -------
  init_struct.memory0_addr        = (uint32_t)rx_buffer;
  init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;

  /* 初始化 DMA single data mode initialize */
  dma_single_data_mode_init(USART1_RX_DMA_PERIPH_CH, &init_struct);

  /* 选择指定通道子集 DMA channel peripheral select** */
  dma_channel_subperipheral_select(USART1_RX_DMA_PERIPH_CH, USART1_RX_DMA_CH_SUB);

  // 通知DMA搬运数据（不会阻塞）
  dma_channel_enable(USART1_RX_DMA_PERIPH_CH);

}
#endif

void USART1_init() {

#if USART1_TX_DMA_ENABLE
  USART1_TX_DMA_init();
#endif

#if USART1_RX_DMA_ENABLE
  USART1_RX_DMA_init();
#endif


  // GPIO ---------------------------------------------------
  // PA9: TX   PA10: RX
  // RCU时钟
  rcu_periph_clock_enable(USART1_TX_RCU);
  // 初始化GPIO模式, 复用模式
  gpio_mode_set(USART1_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_TX_PIN);
  // 设置输出选项
  gpio_output_options_set(USART1_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART1_TX_PIN);
  // 设置GPIO引脚复用功能 USART1
  gpio_af_set(USART1_TX_PORT, USART1_AF, USART1_TX_PIN);

  // RCU时钟
  rcu_periph_clock_enable(USART1_RX_RCU); // 上边RCU_GPIOA初始化过，这里可以省掉
  // 初始化GPIO模式, 复用模式
  gpio_mode_set(USART1_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_RX_PIN);
  // 设置输出选项
  gpio_output_options_set(USART1_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART1_RX_PIN);
  // 设置GPIO引脚复用功能 USART1
  gpio_af_set(USART1_RX_PORT, USART1_AF, USART1_RX_PIN);

  // USART ---------------------------------------------------
  // RCU时钟
  rcu_periph_clock_enable(RCU_USART1);

  // (重置)
  usart_deinit(USART1);
  // 配置usart参数: 波特率，数据个数，校验位，停止位
  usart_baudrate_set(USART1, USART1_BAUDRATE);						// 波特率，必选
  usart_word_length_set(USART1, USART_WL_8BIT); // 数据个数, 默认8bit
  usart_parity_config(USART1, USART_PM_NONE);    // 奇偶校验， 默认无校验位
  usart_stop_bit_set(USART1, USART_STB_1BIT);		// 停止位，默认1bit

  usart_data_first_config(USART1, USART_MSBF_LSB); // 大小端模式： 默认小端, 先发地位 USART_MSBF_LSB

#if USART1_TX_DMA_ENABLE
  // 开启DMA发送功能
  usart_dma_transmit_config(USART1, USART_TRANSMIT_DMA_ENABLE);
#endif

#if USART1_RX_DMA_ENABLE
  // 开启DMA接收功能
  usart_dma_receive_config(USART1, USART_RECEIVE_DMA_ENABLE);
#endif

  // 开启数据发送功能
  usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
  // 开启数据接收功能
  usart_receive_config(USART1, USART_RECEIVE_ENABLE);

  // NVIC中断配置-----------------------------------------------
  nvic_irq_enable(USART1_IRQn, USART1_IRQ_PRIORIY);

#if !USART1_RX_DMA_ENABLE
  // 启用中断 (读数据buffer不为空中断)
  usart_interrupt_enable(USART1, USART_INT_RBNE);
#endif

  // 启用空闲中断
  usart_interrupt_enable(USART1, USART_INT_IDLE);

  // 启用usart
  usart_enable(USART1);

//	printf("usart_data_addr: 0x%X \n", (uint32_t)(USART_DATA(USART1)));
}

// Hello123

// 发送1字节数据
void USART1_send_byte(uint8_t data) {

#if USART1_TX_DMA_ENABLE
  // 使用DMA发送
  USART1_send_data(&data, 1);
#else
  //通过USART发送
  usart_data_transmit(USART1, data);
  // 阻塞式判断缓冲区是否已经空了
  while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
#endif

}

// 发送字节数组
void USART1_send_data(uint8_t* data, uint32_t len) {
  // 通过DMA实现数据的发送

#if USART1_TX_DMA_ENABLE
  // 源地址
  dma_memory_address_config(USART1_TX_DMA_PERIPH_CH, DMA_MEMORY_0, (uint32_t)data);
  // 数据长度
  dma_transfer_number_config(USART1_TX_DMA_PERIPH_CH, len);
  // 通知DMA搬运数据（不会阻塞）
  dma_channel_enable(USART1_TX_DMA_PERIPH_CH);

  // 等待数据传输完毕
  while(RESET == dma_flag_get(USART1_TX_DMA_PERIPH_CH, DMA_FLAG_FTF));
  // 清理标记，避免下次无法发送
  dma_flag_clear(USART1_TX_DMA_PERIPH_CH, DMA_FLAG_FTF);

#else

  while(data && len) {
    // 发送一个字节数据
    USART1_send_byte((uint8_t)(*data));
    // 将字符指针往后移一位，指向下一个字符
    data++;
    // 长度减一
    len--;
  }
#endif
}


// 发送字符串
void USART1_send_string(char *data) {

#if USART1_TX_DMA_ENABLE
  // 使用DMA发送
  USART1_send_data((uint8_t *)data, strlen(data));

#else

  //遇到停止符0结束
  // 1. data不为空， 2. 数据不是停止符'\0'
  while(data && *data) {
    USART1_send_byte((uint8_t)(*data));
    // 将字符指针往后移一位，指向下一个字符
    data++;
  }
#endif

}


/*
1. USART1的中断函数只有一个
2. 触发中断的原因，有多个
3. 需要根据flag区分是什么原因触发的
*/
void USART1_IRQHandler() {

#if USART1_RX_DMA_ENABLE

//	printf("USART1_IRQHandler\n");
  if (SET == usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE)) {
    // 闲置了
    // 读一次寄存器，读出来的值没有用，否则IDLE的flag一直触发
    usart_data_receive(USART1);


    // 获取DMA读到的数据个数 1024 byte
    /* 剩余的数据数量 get the number of remaining data to be transferred by the DMA */
    rx_cnt = USART1_RX_BUFFER_LEN - dma_transfer_number_get(USART1_RX_DMA_PERIPH_CH);

    // 可选：添加字符串结束标记，避免打印出错
    rx_buffer[rx_cnt] = '\0'; // 等同于0


#if USART1_RECV_CALLBACK
    USART1_on_recv(rx_buffer, rx_cnt);
#endif

    // 停止搬运
    dma_channel_disable(USART1_RX_DMA_PERIPH_CH);
    // 清理FTF数据传输完毕标记
    dma_flag_clear(USART1_RX_DMA_PERIPH_CH, DMA_FLAG_FTF);
    // 重置计数器(不需要）
//		dma_transfer_number_config(USART1_RX_DMA_PERIPH_CH, RX_BUFFER_LEN);
    // 启用搬运
    dma_channel_enable(USART1_RX_DMA_PERIPH_CH);

  }

#else

  /***************** 传统的数据接收CPU逐个处理 **************/

  // 判定flag标记
  if (SET == usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)) {
    // 接收到数据
    //		printf("FLAG_RBNE: %d\n", rx_cnt);
    // 清除标记，方便串口电路接收下一个字节数据
    usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);

    // 获取寄存器里的数据
    uint8_t data = usart_data_receive(USART1);

    // 缓存到buffer
    rx_buffer[rx_cnt++] = data;
    if (rx_cnt >= USART1_RX_BUFFER_LEN) {
      rx_cnt = 0;	// 防止缓存区溢出
    }
    // 原样返回（测试）
//    USART1_send_byte(data);
		
#if USART1_RECV_BYTE_CALLBACK
    USART1_on_byte_recv(data);
#endif
  }

  if (SET == usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE)) {
    // 闲置了
    //		printf(">IDLE<: %d\n", rx_cnt);
    // 读一次寄存器，读出来的值没有用，否则IDLE的flag一直触发
    usart_data_receive(USART1);

    // 可选：添加字符串结束标记，避免打印出错
    rx_buffer[rx_cnt] = '\0'; // 等同于0
    //		rx_buffer[rx_cnt] = 0;

    // "abc123"0	字符串打印时，遇到0，会自动停止
    // "hhh"0
#if USART1_RECV_CALLBACK
    USART1_on_recv(rx_buffer, rx_cnt);
#endif

    // 可选：重置缓存区的内容全部为零
    memset(rx_buffer, 0, rx_cnt);
    // 清理缓冲区
    rx_cnt = 0;

  }

#endif
}
