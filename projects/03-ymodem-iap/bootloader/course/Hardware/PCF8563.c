#include "PCF8563.h"


void PCF8563_init() {

}
// 十位取出左移4位 + 个位 (10进制转换得到BCD数)
#define WRITE_BCD(val) 	((val / 10) << 4) + (val % 10)

// 将高4位乘以10 + 低四位 (BCD转换得到10进制数)
#define READ_BCD(val) 	(val >> 4) * 10 + (val & 0x0F) 

#define NUMBER 7

void PCF8563_set_clock(Clock_t c) {
  u8 p[NUMBER];				// 用于存数据的数组
  u8 C; // 0代表本世纪，1代表下个世纪
  // 将十进制数值转成BCD格式（1个字节）

  // 秒：VL	1 1 1 - 0 0 0 0 BCD格式
  p[0] = WRITE_BCD(c.second);

  // 分： X	1 1 1 - 0 0 0 0 BCD格式
  p[1] = WRITE_BCD(c.minute);
	
  // 时： X	X 1 1 - 0 0 0 0 BCD格式
  p[2] = WRITE_BCD(c.hour);
	
  // 天： X	X 1 1 - 0 0 0 0 BCD格式
  p[3] = WRITE_BCD(c.day);
	
  // 周： X	X X X - X 0 0 0 BCD格式
  p[4] = c.week;

  // 世纪C
  C = c.year < 2100 ? 0 : 1;
  // 月： C	X X 1 - 0 0 0 0 BCD格式
  p[5] = (C << 7) + WRITE_BCD(c.month);

  // 年： 1	1 1 1 - 0 0 0 0 BCD格式
  p[6] = ((c.year % 100 / 10) << 4) + (c.year % 10);

	// 写到Slave从设备寄存器里
  I2C_WRITE(PCF8563_ADDR, PCF8563_REG, p, NUMBER);
}


void PCF8563_get_clock(Clock_t* c) {
  u8 p[NUMBER];				// 用于存数据的数组
  u8 C; // 0代表本世纪，1代表下个世纪

	// 读取Slave从设备寄存器数据
  // I2C_READ(u8 dev_addr, u8 mem_addr, u8 *p, u8 number);
  I2C_READ(PCF8563_ADDR, PCF8563_REG, p, NUMBER);

  // 数据读取的格式是BCD格式, 转成十进制
  // 秒：VL	1 1 1 - 0 0 0 0 转成十进制
  c->second = ((p[0] >> 4) & 0x07) * 10 + (p[0] & 0x0F);

  // 分： X	1 1 1 - 0 0 0 0 转成十进制
  c->minute = ((p[1] >> 4) & 0x07) * 10 + (p[1] & 0x0F);

  // 时： X	X 1 1 - 0 0 0 0 转成十进制
  c->hour 	= ((p[2] >> 4) & 0x03) * 10 + (p[2] & 0x0F);

  // 天： X	X 1 1 - 0 0 0 0 转成十进制
  c->day 	  = ((p[3] >> 4) & 0x03) * 10 + (p[3] & 0x0F);

  // 周： X	X X X - X 0 0 0 转成十进制
  c->week   = p[4] & 0x07;															  // 0 周日

  // 世纪C
  // 月： C	X X 1 - 0 0 0 0 转成十进制
  c->month  = ((p[5] >> 4) & 0x01) * 10 + (p[5] & 0x0F); // 0 一月
  C 		 = p[5] >> 7;						// 0 -> 20xx年, 1 -> 21xx年 C Century

  // 年： 1	1 1 1 - 0 0 0 0 转成十进制
  c->year 	 = ((p[6] >> 4) & 0x0F) * 10 + (p[6] & 0x0F);
  // 加上世纪信息
  c->year  += (C == 0 ? 2000 : 2100);
}

// 设置闹铃
void PCF8563_set_alarm(Alarm_t a){
	u8 p[4];
	
	// 分 M 1 1 1 - 0 0 0 0  enable->0x00, disable->0x80
	if(a.minute == -1){
		p[0] = 0x80; // 如果-1，禁用
	}else {
		p[0] = ((a.minute / 10) << 4) + (a.minute % 10);
	}
	
	// 时 H x 1 1 - 0 0 0 0  enable->0x00, disable->0x80
	if(a.hour == -1){
		p[1] = 0x80;	// 如果-1，禁用
	}else {
		p[1] = ((a.hour / 10) << 4) + (a.hour % 10);
	}
	
	// 天 D x 1 1 - 0 0 0 0  enable->0x00, disable->0x80
	if(a.day == -1){
		p[2] = 0x80; 	// 如果-1，禁用
	}else {
		p[2] = ((a.day / 10) << 4) + (a.day % 10);
	}
	
	// 周 W x x x - x 0 0 0  enable->0x00, disable->0x80
	if(a.weekday == -1) {
		p[3] = 0x80;	// 如果-1，禁用
	}else {
		p[3] = a.weekday;
	}
	
  I2C_WRITE(PCF8563_ADDR, 0x09, p, 4);
}

// 启用闹铃
void PCF8563_enable_alarm(u8 enable){
	u8 cs2;
		// 读取原来字节cs2 0x01
  I2C_READ(PCF8563_ADDR, 0x01, &cs2, 1);
	// 清理Alarm标记，AF设置为0，下一次闹钟到点时，才能触发
	cs2 &=~(1 << 3); // 0x08
	// 开启Alarm中断，AIE设置为1，启用Alarm
	if(enable){
		cs2 |= (1 << 1); // 0x02 置1
	}else {
		cs2 &=~(1 << 1); // 0x02 置0
	}
	// 写回去
  I2C_WRITE(PCF8563_ADDR, 0x01, &cs2, 1);
}

// 清理闹钟标记
void PCF8563_clear_alarm_flag(){
	u8 cs2;
	// 读取原来字节cs2 0x01
  I2C_READ(PCF8563_ADDR, 0x01, &cs2, 1);
	// 清理Alarm标记，AF设置为0，下一次闹钟到点时，才能触发
	cs2 &=~(1 << 3); // 0x08
	// 写回去
  I2C_WRITE(PCF8563_ADDR, 0x01, &cs2, 1);
}

// 设置定时器Timer
void PCF8563_set_timer(TimerFreq freq, u8 countdown){
	u8 p;
	
	// 启用Timer频率，TE设置为1. 设置运行频率0x01 -> 64Hz
	p = freq + 0x80;
  I2C_WRITE(PCF8563_ADDR, 0x0E, &p, 1);
	// 设置计数值(每个周期的计数值)
	p = countdown;
  I2C_WRITE(PCF8563_ADDR, 0x0F, &p, 1);
}

// 启用定时器
void PCF8563_enable_timer(u8 enable){
	u8 cs2;
	// 读取原来字节cs2 0x01
  I2C_READ(PCF8563_ADDR, 0x01, &cs2, 1);
	// 清理Timer标记，TF设置为0，下一次闹钟到点时，才能触发
	cs2 &=~(1 << 2); // 0x04
	// 开启Timer中断，TIE设置为1，启用Timer
	if(enable){
		cs2 |= (1 << 0); // 0x01
	}else {
		cs2 &=~(1 << 0); // 0x01
	}
	// 写回去
  I2C_WRITE(PCF8563_ADDR, 0x01, &cs2, 1);
}

// 清理定时器标记
void PCF8563_clear_timer_flag(){
	u8 cs2;
	// 读取原来字节cs2 0x01
  I2C_READ(PCF8563_ADDR, 0x01, &cs2, 1);
	// 清理Timer标记，TF设置为0，下一次闹钟到点时，才能触发
	cs2 &=~(1 << 2); // 0x04
	// 写回去
  I2C_WRITE(PCF8563_ADDR, 0x01, &cs2, 1);
}

void ext_int3_call(void) {
	// 0b 0000 1010  = 0x08+0x02
	// 0b 0000 0101  = 0x04+0x01
	//&0b 0000 1000
	
	u8 cs2;
	// 读取原来字节cs2 0x01
  I2C_READ(PCF8563_ADDR, 0x01, &cs2, 1);
	
#if USE_ALARM
	// AlarmFlag AF && AIE
	// 当中断触发时的实现逻辑	
	if((cs2 & 0x08) && (cs2 & 0x02)){
		PCF8563_on_alarm();
		// 清理标记 Alarm
		PCF8563_clear_alarm_flag();
	}
#endif
	
#if USE_TIMER
	// TimerFlag TF && TIE
	if((cs2 & 0x04) && (cs2 & 0x01)){
		PCF8563_on_timer();
		// 清理标记 Timer
		PCF8563_clear_timer_flag();
	}
#endif
}


