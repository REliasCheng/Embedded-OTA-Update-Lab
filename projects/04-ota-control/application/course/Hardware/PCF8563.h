#ifndef __PCF8563_H__
#define __PCF8563_H__

#include "gd32f4xx.h"
#include "I2C.h"

#ifndef u8
#define u8    uint8_t
#endif

#ifndef u16
#define u16   uint16_t
#endif

#ifndef u32
#define u32   uint32_t
#endif

#define I2C_WRITE(addr, reg, p, len)  I2C_write(addr, reg, p, len)

#define I2C_READ(addr, reg, p, len)   I2C_read(addr, reg, p, len)

#define USE_ALARM		1
#define USE_TIMER	  1

//#define PCF8563_ADDR 0xA2 // 设备写地址 0xA2
#define PCF8563_ADDR 0x51 // 设备地址 0xA2
#define PCF8563_REG  0x02 // 寄存器地址（开始位置, 秒 寄存器）

// 时钟结构体：秒、分、时、星期、日、月、年、世纪
typedef struct{

	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 week;
	
} Clock_t;

// 闹铃结构体：分、时、天、周
typedef struct {

	char minute;
	char hour;
	char day;
	char weekday;
} Alarm_t;

// 定时器频率枚举
typedef enum {
	
	HZ4096 = 0x00,
	HZ64 	 = 0x01,
	HZ1 	 = 0x02,
	HZ1_64 = 0x03,
	
} TimerFreq; // Frequence

void PCF8563_init();

void PCF8563_set_clock(Clock_t c);

void PCF8563_get_clock(Clock_t* c);

// 设置闹铃
void PCF8563_set_alarm(Alarm_t alarm);

// 启用闹铃 1->enable  0->disable
void PCF8563_enable_alarm(u8 enable);

// 清理闹钟标记
void PCF8563_clear_alarm_flag();


// 设置定时器Timer
void PCF8563_set_timer(TimerFreq freq, u8 countdown);

// 启用定时器 1->enable  0->disable
void PCF8563_enable_timer(u8 enable);

// 清理定时器标记
void PCF8563_clear_timer_flag();

#if USE_ALARM
extern void PCF8563_on_alarm();
#endif

#if USE_TIMER
extern void PCF8563_on_timer();
#endif

#endif