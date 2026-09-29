#include "bsp_w25qxx.h"

void W25Qxx_init_config(void)
{
	
  //开启CS引脚时钟
  rcu_periph_clock_enable(RCU_GPIOF);
  //配置CS引脚模式
  gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6);
  //配置CS输出模式
  gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
  //W25Q64不选中
  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);


}

/******************************************************************
 * 函 数 名 称：W25Qxx_readID
 * 函 数 说 明：读取W25Q64的厂商ID和设备ID
 * 函 数 形 参：无
 * 函 数 返 回：设备正常返回EF16
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint16_t W25Qxx_readID(void)
{
  uint16_t  temp = 0;
  gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);

  spi_read_write_byte(0x90);//发送读取ID命令
  spi_read_write_byte(0x00);
  spi_read_write_byte(0x00);
  spi_read_write_byte(0x00);
  //接收数据
  temp |= spi_read_write_byte(0xFF)<<8;
  temp |= spi_read_write_byte(0xFF);

  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
  return temp;
}



/**********************************************************
 * 函 数 名 称：W25Qxx_wait_busy
 * 函 数 功 能：判断W25Q64是否忙
 * 传 入 参 数：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Qxx_wait_busy(void)
{
  unsigned char byte = 0;
  do
  {
    gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
    spi_read_write_byte(0x05);
    byte = spi_read_write_byte(0Xff);
    gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
  } while( ( byte & 0x01 ) == 1 );
}

/**********************************************************
 * 函 数 名 称：W25Qxx_write_enable
 * 函 数 功 能：发送写使能
 * 传 入 参 数：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Qxx_write_enable(void)
{
  gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
  spi_read_write_byte(0x06);
  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
}

/**********************************************************
 * 函 数 名 称：W25Qxx_erase_sector
 * 函 数 功 能：擦除一个扇区
 * 传 入 参 数：addr=擦除的扇区号
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：addr=擦除的扇区号，范围=0~15
**********************************************************/
void W25Qxx_erase_sector(uint32_t addr)
{
  addr *= 4096;
  W25Qxx_write_enable();  //写使能
  W25Qxx_wait_busy();     //判断忙
  gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
  spi_read_write_byte(0x20);
  spi_read_write_byte((uint8_t)((addr)>>16));
  spi_read_write_byte((uint8_t)((addr)>>8));
  spi_read_write_byte((uint8_t)addr);
  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
  //等待擦除完成
  W25Qxx_wait_busy();
}


/**********************************************************
 * 函 数 名 称：W25Qxx_write
 * 函 数 功 能：写数据到W25Q64进行保存
 * 传 入 参 数：buffer=写入的数据内容	addr=写入地址	numbyte=写入数据的长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Qxx_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{ //0x02e21
  unsigned int i = 0;
  W25Qxx_erase_sector(addr/4096);//擦除扇区数据
  W25Qxx_write_enable();//写使能
  W25Qxx_wait_busy(); //忙检测
  //写入数据
  gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
  spi_read_write_byte(0x02);
  spi_read_write_byte((uint8_t)((addr)>>16));
  spi_read_write_byte((uint8_t)((addr)>>8));
  spi_read_write_byte((uint8_t)addr);
  for(i=0; i<numbyte; i++)
  {
    spi_read_write_byte(buffer[i]);
  }
  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
  W25Qxx_wait_busy(); //忙检测
}

/**********************************************************
 * 函 数 名 称：W25Qxx_read
 * 函 数 功 能：读取W25Q64的数据
 * 传 入 参 数：buffer=读出数据的保存地址  read_addr=读取地址		read_length=读去长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void W25Qxx_read(uint8_t* buffer,uint32_t read_addr,uint16_t read_length)
{
  uint16_t i;
  gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
  spi_read_write_byte(0x03);
  spi_read_write_byte((uint8_t)((read_addr)>>16));
  spi_read_write_byte((uint8_t)((read_addr)>>8));
  spi_read_write_byte((uint8_t)read_addr);
  for(i=0; i<read_length; i++)
  {
    buffer[i]= spi_read_write_byte(0XFF);
  }
  gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
}

