#include "SPI0.h"


#if SPI0_SOFT

// =============================== SPI 软实现=====================

void SPI0_init(void) {

  rcu_periph_clock_enable(SPI0_SCL_RCU);
  gpio_mode_set(SPI0_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI0_SCL_PIN);
  gpio_output_options_set(SPI0_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_SCL_PIN);

  rcu_periph_clock_enable(SPI0_MOSI_RCU);
  gpio_mode_set(SPI0_MOSI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI0_MOSI_PIN);
  gpio_output_options_set(SPI0_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_MOSI_PIN);

  // MISO
  rcu_periph_clock_enable(SPI0_MISO_RCU);
  gpio_mode_set(SPI0_MISO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, SPI0_MISO_PIN);

  // 拉高
  gpio_bit_write(SPI0_SCL_PORT, SPI0_SCL_PIN, SET);
  gpio_bit_write(SPI0_MOSI_PORT, SPI0_MOSI_PIN, SET);
}

// 写数据 （MSB大端模式 1101 0011）
void SPI0_write(uint8_t dat) {
  for(uint8_t i = 0; i < 8; i++) {
    // 拉低SCL
    SCL(0);

    // 改变数据（输出）
    MOSI(dat & 0x80);

    // 左移一位
    dat <<= 1;

    // 拉高SCL
    SCL(1);
  }
}

// 读数据 （MSB大端模式 1101 0011）
uint8_t SPI0_read() {
  // 0000 0000 <- 1101 0011
  uint8_t i, read = 0x00;
  for(i = 0; i < 8; i++) {
    // SCL拉低
    SCL(0);

    read <<= 1;
    if(MISO()) read++;

    // SCL拉高
    SCL(1);
  }

  return read;
}
// 写数据&读数据
uint8_t SPI0_write_read(uint8_t dat){
  uint8_t read = 0x00;
  for(uint8_t i = 0; i < 8; i++) {
    // 拉低SCL
    SCL(0);

    // 改变数据（输出）
    MOSI(dat & 0x80);

    // 读数据
    read <<= 1;
    if(MISO()) read++;
    
    // 左移一位
    dat <<= 1;

    // 拉高SCL
    SCL(1);
  }
  return read;
}
#else


// =============================== SPI 硬实现=====================
void SPI0_init(void) {
  // GPIO ---------------------------------
  // SCL, MOSI, MISO (CS和设备业务挂钩，不用在这里实现)
  // SCL
  rcu_periph_clock_enable(SPI0_SCL_RCU);
  gpio_mode_set(SPI0_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI0_SCL_PIN);
  gpio_output_options_set(SPI0_SCL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_SCL_PIN);
  gpio_af_set(SPI0_SCL_PORT, GPIO_AF_5, SPI0_SCL_PIN);

  // MOSI
  rcu_periph_clock_enable(SPI0_MOSI_RCU);
  gpio_mode_set(SPI0_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI0_MOSI_PIN);
  gpio_output_options_set(SPI0_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, SPI0_MOSI_PIN);
  gpio_af_set(SPI0_MOSI_PORT, GPIO_AF_5, SPI0_MOSI_PIN);

  // MISO
  rcu_periph_clock_enable(SPI0_MISO_RCU);
  gpio_mode_set(SPI0_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SPI0_MISO_PIN);
  gpio_af_set(SPI0_MISO_PORT, GPIO_AF_5, SPI0_MISO_PIN);


  // SPI ----------------------------------
  // rcu
  rcu_periph_clock_enable(RCU_SPI0);
  spi_parameter_struct spi_struct;
  /* 给结构体填充默认值 initialize the parameters of SPI struct with default values */
  spi_struct_para_init(&spi_struct);
  
  spi_struct.device_mode          = SPI_MASTER;                // 设备模式：主机
  spi_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;  // 传输模式： 默认
  spi_struct.frame_size           = SPI_FRAMESIZE_8BIT;        // 每帧8bit
  spi_struct.nss                  = SPI_NSS_SOFT;              // 软片选
  spi_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;   // CPOL:1, CPHA:1
  spi_struct.prescale             = SPI_PSC_2;                 // 分频系数：168M / 2 = 84M
  spi_struct.endian               = SPI_ENDIAN_MSB;            // 大小端模式: 大端
  
  /* 根据结构体参数初始化SPI0 initialize SPI parameter */
  spi_init(SPI0, &spi_struct);
  /* enable SPI */
  spi_enable(SPI0);


}

// 写数据 (写的时候，也要取数据)
void SPI0_write(uint8_t dat) {
  /* 循环等待发送缓冲区，直到为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
  /* 通知外设电路，发送数据 SPI transmit data*/
  spi_i2s_data_transmit(SPI0, dat); // MOSI
  
  /* 循环等待接收缓冲区，直到不为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
  spi_i2s_data_receive(SPI0);
}

// 读数据 (读的时候，也要写任意数据)
uint8_t SPI0_read() {
  /* 循环等待发送缓冲区，直到为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
  /* 通知外设电路，发送数据 SPI transmit data*/
  spi_i2s_data_transmit(SPI0, 0x00); // MOSI
  
  /* 循环等待接收缓冲区，直到不为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
  return spi_i2s_data_receive(SPI0);
}


// 写数据&读数据
uint8_t SPI0_write_read(uint8_t dat){
  /* 循环等待发送缓冲区，直到为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
  /* 通知外设电路，发送数据 SPI transmit data*/
  spi_i2s_data_transmit(SPI0, dat); // MOSI
  
  /* 循环等待接收缓冲区，直到不为空SET */
  while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
  return spi_i2s_data_receive(SPI0);
}

#endif