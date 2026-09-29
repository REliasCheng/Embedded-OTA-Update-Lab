#include "I2C0.h"

#if USE_I2C0

#if I2C0_SOFT

#include "systick.h"

#if I2C0_SPEED >= 400000UL
  #define DELAY()   delay_1us(1)
#else
  #define DELAY()   delay_1us(5)
#endif

#define SDA(bit)  gpio_bit_write(I2C0_SDA_PORT, I2C0_SDA_PIN, (bit) ? SET : RESET)
#define SCL(bit)  gpio_bit_write(I2C0_SCL_PORT, I2C0_SCL_PIN, (bit) ? SET : RESET)

#define I2C0_SDA_OUT() gpio_mode_set(I2C0_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C0_SDA_PIN);
#define I2C0_SDA_IN()  gpio_mode_set(I2C0_SDA_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, I2C0_SDA_PIN);
// 读取SDA电平
#define I2C0_SDA_STATE() gpio_input_bit_get(I2C0_SDA_PORT, I2C0_SDA_PIN)

// 初始化I2C的两个总线GPIO为开漏输出
void I2C0_init(){
  // 软实现：CPU操作GPIO
  // 硬实现：CPU操作I2C的寄存器，AF复用，初始化外设
  
  // 标准模式 100Kbits/s, 快速模式 400Kbits/s
  // 100Kbits/s -> 100 000 bit / 1 000 000us -> 1bit/10us -> 5us
  // 400Kbits/s -> 400 000 bit / 1 000 000us -> 4bit/10us -> 1us+
  
  // 休眠时间1us -> 1bit需要2us  -> 4bit需要8us
  
  // PB6 - SCL
  rcu_periph_clock_enable(I2C0_SCL_RCU);
  gpio_mode_set(I2C0_SCL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C0_SCL_PIN);
  gpio_output_options_set(I2C0_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, I2C0_SCL_PIN);
  // PB7 - SDA
  rcu_periph_clock_enable(I2C0_SDA_RCU);
  gpio_mode_set(I2C0_SDA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, I2C0_SDA_PIN);
  gpio_output_options_set(I2C0_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, I2C0_SDA_PIN);
}

// 开始信号
static void start();
// 设备地址，寄存器地址，数据
static void send(uint8_t dat);
// 接收1个字节
static uint8_t recv();
// 发送ack响应
static void send_ack();
// 发送nack响应
static void send_nack();

// 等待响应，返回值 > 0, 出现异常
static uint8_t wait_ack();
// 停止信号
static void stop();


/**********************************************************
 * @brief 写数据到I2C设备指定寄存器
 * @param  addr 设备地址 (写地址的前7位)
 * @param  reg  寄存器地址
 * @param  dat  字节数组
 * @param  len  数据长度
 * @return 0成功，1 设备不存在, 2 寄存器不存在, 3 数据无响应
 **********************************************************/
uint8_t I2C0_write(uint8_t addr, uint8_t reg, uint8_t* dat, uint32_t len){
  
  // 开始
  start();
  
  // 发送设备地址 (写地址)
  send((addr << 1) | 0);
  // 等待响应
  if(wait_ack()) return 1;
  
  // 发送寄存器地址
  send(reg);
  // 等待响应
  if(wait_ack()) return 2;
  
  // 循环发送所有的字节
  for(uint32_t i = 0; i < len; i++){
    // 发送数据
    send(dat[i]);
    // 等待响应
    if(wait_ack()) return 3;
  }
  
  // 停止
  stop();
  
  return 0;
}

uint8_t I2C0_write2(uint8_t addr, uint8_t reg, uint8_t* dat, uint32_t offset, uint32_t len){
  
  // 开始
  start();
  
  // 发送设备地址 (写地址)
  send((addr << 1) | 0);
  // 等待响应
  if(wait_ack()) return 1;
  
  // 发送寄存器地址
  send(reg);
  // 等待响应
  if(wait_ack()) return 2;
  
  // 循环发送所有的字节
//  for(uint32_t i = 0; i < len; i++){
//    // 发送数据
//    send(dat[i * offset]);
//    // 等待响应
//    if(wait_ack()) return 3;
//  }
  while(len--){
    send(*dat);
    dat += offset;
    // 等待响应
    if(wait_ack()) return 3;
  }
  
  // 停止
  stop();
  
  return 0;
}

/**********************************************************
 * @brief 从I2C设备指定寄存器读取数据
 * @param  addr 设备地址  (写地址的前7位)   0x51
                写地址 (addr << 1) | 0     0xA2
                读地址 (addr << 1) | 1     0xA3
 * @param  reg  寄存器地址
 * @param  dat  字节数组
 * @param  len  数据长度
 * @return 0成功，1 设备不存在, 2 寄存器不存在, 3 数据无响应
 * @return 
 **********************************************************/
uint8_t I2C0_read(uint8_t addr, uint8_t reg, uint8_t* dat, uint32_t len){
  uint8_t addr_write = (addr << 1) | 0;  
  uint8_t addr_read  = (addr << 1) | 1;
  
  // 开始
  start();
  
  // 发送设备地址 (写地址)
  send(addr_write);
  // 等待响应
  if(wait_ack()) return 1;
  
  // 发送寄存器地址
  send(reg);
  // 等待响应
  if(wait_ack()) return 2;
  
  // 开始
  start();
  // 发送设备地址 (读地址)
  send(addr_read);
  // 等待响应
  if(wait_ack()) return 3;
  
  /******************* 循环接收数据 *****************/
  for(uint32_t i = 0; i < len; i++){
    // 接收1个字节(8bit)
    dat[i] = recv();
    
    if(i != len - 1){
      // 发送响应ACK
      send_ack();
    }else {
      // 发送空响应NACK
      send_nack();
    }
  }
  
  /**************************************************/
  
  // 停止
  stop();
  
  return 0;
}


// 开始信号
static void start(){
  I2C0_SDA_OUT();
  // 拉高SDA
  SDA(1);
  DELAY();
  
  // 拉高SCL
  SCL(1);
  DELAY();
  
  // 拉低SDA
  SDA(0);
  DELAY();
  
  // 拉低SCL
  SCL(0);
  DELAY();
}

// 设备地址，寄存器地址，数据
static void send(uint8_t dat){
  I2C0_SDA_OUT();
  // 8bit 先发高位
  // 1101 0010
  // 101 0010  左移1位
  // 01 0010   左移1位
  for(uint8_t i = 0; i < 8; i++){
    if(dat & 0x80){
      SDA(1); // 高电平
    }else {
      SDA(0); // 低电平
    }
    DELAY();
    
    SCL(1);
    DELAY();
    SCL(0);
    DELAY();
    
    // 所有内容左移1位
    dat <<= 1;
  }
}

// 接收1个字节
static uint8_t recv(){
  // 释放SDA控制权
  I2C0_SDA_IN();
  uint8_t cnt = 8; // 1byte = 8bit
  uint8_t byte = 0x00; // 空容器接收数据
  
  while(cnt--){   // 接收一个bit（先收高位）
    // SCL拉低
    SCL(0); 
    DELAY(); // 等待从设备准备数据
    
    SCL(1);  // 设置数据有效性
    // 0000 0000 -> 1001 1011
    // 0000 0001    8
    // 0000 0010    7
    // 0000 0100    6
    // 0000 1001    5
    // 0001 0011    4
    // 0010 0110    3
    // 0100 1101    2
    // 1001 1011    1
    
    byte <<= 1;
    
    if(I2C0_SDA_STATE()) byte++;
    
    // SCL在高电平Delay一会儿，进入下一个循环
    DELAY();
  }
  // SCL恢复低电平, 不能少
  SCL(0);
  
  return byte;
}

// 发送ack响应
static void send_ack(){
  // 主机获取SDA控制权
  I2C0_SDA_OUT();
  // SDA拉低
  SDA(0);
  DELAY();
  
  // SCL拉高
  SCL(1);
  DELAY();
  
  // SCL拉低
  SCL(0);
  DELAY();
  
}
// 发送nack响应
static void send_nack(){
  // 主机获取SDA控制权
  I2C0_SDA_OUT();
  // SDA拉高
  SDA(1);
  DELAY();
  
  // SCL拉高
  SCL(1);
  DELAY();
  
  // SCL拉低
  SCL(0);
  DELAY();

}

// 等待响应，返回值 > 0, 出现异常
static uint8_t wait_ack(){
  I2C0_SDA_OUT();
  // 拉高SDA
  SDA(1);
  DELAY();
  
  // 拉高SCL， 转交控制权（SDA转为输入模式）
  SCL(1);
  I2C0_SDA_IN();
  DELAY();
  
  // 判定SDA电平状态
  if(I2C0_SDA_STATE() == RESET){
    // 从设备拉低了SDA，响应成功, 获取SDA权限
    SCL(0);
    I2C0_SDA_OUT();
  } else {
    // 无人应答, 获取SDA权限，直接结束
    stop();
    return 1;
  }
  
  return 0;
}
// 停止信号
static void stop(){
  I2C0_SDA_OUT();
  
  // 拉低SCL
  SCL(0);
  // 拉低SDA
  SDA(0);
  DELAY();
  
  // 拉高SCL
  SCL(1);
  DELAY();
  
  // 拉高SDA
  SDA(1);
  DELAY();
}


#else

#define I2C0RCU     RCU_I2C0
#define i2cx        I2C0
// PB6 - SCL
// PB7 - SDA
void I2C0_init(){

  // 软实现：CPU操作GPIO
  // 硬实现：CPU操作I2C的寄存器，AF复用，初始化外设
  // 标准模式 100Kbits/s, 快速模式 400Kbits/s
  // 100Kbits/s -> 100 000 bit / 1 000 000us -> 1bit/10us -> 5us
  // 400Kbits/s -> 400 000 bit / 1 000 000us -> 4bit/10us -> 1us+
  
  // GPIO -----------------------------------------
  // SCL
  rcu_periph_clock_enable(I2C0_SCL_RCU);
  gpio_mode_set(I2C0_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C0_SCL_PIN);
  gpio_output_options_set(I2C0_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, I2C0_SCL_PIN);
  gpio_af_set(I2C0_SCL_PORT, GPIO_AF_4, I2C0_SCL_PIN);
  // SDA
  rcu_periph_clock_enable(I2C0_SDA_RCU);
  gpio_mode_set(I2C0_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, I2C0_SDA_PIN);
  gpio_output_options_set(I2C0_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, I2C0_SDA_PIN);
  gpio_af_set(I2C0_SDA_PORT, GPIO_AF_4, I2C0_SDA_PIN);
  
  // I2C  ------------------------------------------
  rcu_periph_clock_enable(I2C0RCU);
  /* 重置 reset I2C */
  i2c_deinit(i2cx);
  /* 配置I2C的速度，低高电平的比例 configure I2C clock */
  i2c_clock_config(i2cx, I2C0_SPEED, I2C_DTCY_2);
  /* 配置I2C地址类型 configure I2C address */
  i2c_mode_addr_config(i2cx, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
  /* 启用 enable I2C */
  i2c_enable(i2cx);
  
  /* 收到数据自动回复ACK  whether or not to send an ACK */
  i2c_ack_config(i2cx, I2C_ACK_ENABLE);

}
#define	TIMEOUT	50000

// 等待标记flag变为SET，成功return 0
static uint8_t I2C_wait(uint32_t flag) {
    uint16_t cnt = 0;

    while(!i2c_flag_get(i2cx, flag)) {
        cnt++;
        if(cnt > TIMEOUT) return 1;
    }
    return 0;
}

// 等待标记flag变为RESET, 成功return 0
static uint8_t I2C_waitn(uint32_t flag) {
    uint16_t cnt = 0;

    while(i2c_flag_get(i2cx, flag)) {
        cnt++;
        if(cnt > TIMEOUT) return 1;
    }
		return 0;
}

/* 写数据到I2C设备指定寄存器 */
uint8_t I2C0_write(uint8_t addr, uint8_t reg, uint8_t* dat, uint32_t len){
  
  uint8_t write_addr = addr << 1;
  /************* start ***********************/
  // 等待I2C闲置
  if(I2C_waitn(I2C_FLAG_I2CBSY)) return 1;
  // start
  i2c_start_on_bus(i2cx);
  // 等待I2C主设备成功发送起始信号
  if(I2C_wait(I2C_FLAG_SBSEND)) return 2;
  
  /************* device address **************/
  // 发送设备地址
  i2c_master_addressing(i2cx, write_addr, I2C_TRANSMITTER);
  // 等待地址发送完成
  if(I2C_wait(I2C_FLAG_ADDSEND)) return 3;
  i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);
  
  /************ register address ************/
  // 寄存器地址
  // 等待发送数据缓冲区为空 SET
  if(I2C_wait(I2C_FLAG_TBE)) return 4;
  // 发送数据
  i2c_data_transmit(i2cx, reg);
  // 等待数据发送完成
  if(I2C_wait(I2C_FLAG_BTC)) return 5;
  
  /***************** data ******************/
  // 发送数据
  uint32_t i;
  for(i = 0; i < len; i++) {
      uint32_t d = dat[i];
      // 等待发送数据缓冲区为空
      if(I2C_wait(I2C_FLAG_TBE)) return 6;
      // 发送数据
      i2c_data_transmit(i2cx, d);
      // 等待数据发送完成
      if(I2C_wait(I2C_FLAG_BTC)) return 7;
  }
  /***************** stop ********************/
  // stop
  i2c_stop_on_bus(i2cx);

  if(I2C_waitn(I2C_CTL0(i2cx)&I2C_CTL0_STOP)) return 8;
  
  return 0;
}


/* 从I2C设备指定寄存器读取数据 */
uint8_t I2C0_read(uint8_t addr, uint8_t reg, uint8_t* dat, uint32_t len){
  uint8_t write_addr = (addr << 1) | 0;
  uint8_t  read_addr = (addr << 1) | 1;
  
  /************* start ***********************/
  // 等待I2C空闲
  if(I2C_waitn(I2C_FLAG_I2CBSY)) return 1;
  // 发送启动信号
  i2c_start_on_bus(i2cx);
  // 等待I2C主设备成功发送起始信号
  if(I2C_wait(I2C_FLAG_SBSEND)) return 2;
  
  /************* device address **************/
  // 发送从设备地址（写地址）
  i2c_master_addressing(i2cx, write_addr, I2C_TRANSMITTER);

  if(I2C_wait(I2C_FLAG_ADDSEND)) return 3;
  i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);
  
  /********** register address **************/
  // 等待发送缓冲区为空
  if(I2C_wait(I2C_FLAG_TBE)) return 4;
  // 发送寄存器地址
  i2c_data_transmit(i2cx, reg);
  // 等待发送数据完成	
  if(I2C_wait(I2C_FLAG_BTC)) return 5;
  
  /************* start ***********************/
  // 发送再启动信号
  i2c_start_on_bus(i2cx);
  if(I2C_wait(I2C_FLAG_SBSEND)) return 7;
  
  /************* device address **************/
  // 发送从设备地址（读地址）
  i2c_master_addressing(i2cx, read_addr, I2C_RECEIVER);
  if(I2C_wait(I2C_FLAG_ADDSEND)) return 8;
  i2c_flag_clear(i2cx, I2C_FLAG_ADDSEND);
  
  /************* data **************/
  // ack
  i2c_ack_config(i2cx, I2C_ACK_ENABLE);
  // 接收一个数据后，自动发送ACK
  i2c_ackpos_config(i2cx, I2C_ACKPOS_CURRENT);
  // 确认ACK已启用
  if(I2C_wait(I2C_CTL0(i2cx) & I2C_CTL0_ACKEN)) return 9;

  // 读取数据
  uint32_t i;
  for (i = 0; i < len; i++) {
      if (i == len - 1) {
          // 在读取最后一个字节之前，禁用ACK，配置为自动NACK
          i2c_ack_config(i2cx, I2C_ACK_DISABLE);
      }

      // 等待接收缓冲区不为空
      if(I2C_wait(I2C_FLAG_RBNE)) return 10;
      
      dat[i] = i2c_data_receive(i2cx);
  }
  
  /***************** stop ********************/
  i2c_stop_on_bus(i2cx);
  
  if(I2C_waitn(I2C_CTL0(i2cx)&I2C_CTL0_STOP)) return 11;
  
  return 0;
}

#endif

#endif

