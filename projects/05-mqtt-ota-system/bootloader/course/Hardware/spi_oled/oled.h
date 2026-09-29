#ifndef __OLED_H
#define __OLED_H 

#include "gd32f4xx.h"

#include "SPI.h"

//SCL=SCLK 
//SDA=MOSI
//DC=DC
//CS=CS1
//FS0=MOSI
//CS2=CS2

#ifndef u8
#define u8 uint8_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

#ifndef u32
#define u32 uint32_t
#endif

#define DC_RCU			RCU_GPIOB
#define DC_PORT			GPIOB
#define DC_PIN			GPIO_PIN_8

#define CS1_RCU			RCU_GPIOA
#define CS1_PORT		GPIOA
#define CS1_PIN			GPIO_PIN_15

#define CS2_RCU			RCU_GPIOA
#define CS2_PORT		GPIOA
#define CS2_PIN			GPIO_PIN_4

//-----------------OLED端口定义---------------- 

#define SPI_WRITE(dat)	SPI0_write(dat)
#define	SPI_READ()			SPI0_read()

#define OLED_DC_Clr()  		gpio_bit_write(DC_PORT,DC_PIN, RESET)//DC
#define OLED_DC_Set()  		gpio_bit_write(DC_PORT,DC_PIN, SET)
 		     
#define OLED_CS_Clr()  		gpio_bit_write(CS1_PORT,CS1_PIN, RESET)//CS1
#define OLED_CS_Set()  		gpio_bit_write(CS1_PORT,CS1_PIN, SET)

#define OLED_ROM_CS_Clr()  gpio_bit_write(CS2_PORT,CS2_PIN, RESET)//CS2
#define OLED_ROM_CS_Set()  gpio_bit_write(CS2_PORT,CS2_PIN, SET)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_WR_Byte(u8 dat,u8 cmd);
void OLED_Clear(void);
void OLED_address(u8 x,u8 y);
void OLED_Display_128x64(u8 *dp);
void OLED_Display_16x16(u8 x,u8 y,u8 *dp);
void OLED_Display_8x16(u8 x,u8 y,u8 *dp);
void OLED_Display_5x7(u8 x,u8 y,u8 *dp);
void Send_Command_to_ROM(u8 dat);
u8 Get_data_from_ROM(void);
void OLED_get_data_from_ROM(u8 addrHigh,u8 addrMid,u8 addrLow,u8 *pbuff,u8 DataLen);
void OLED_Display_GB2312_string(u8 x,u8 y,char *text);
void OLED_Display_string_5x7(u8 x,u8 y,char *text);
void OLED_ShowNum(u8 x,u8 y,float num,u8 len);
void OLED_Init(void);
#endif

