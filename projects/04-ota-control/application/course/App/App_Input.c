#include "App.h"
#include "bsp_keys_adv.h"
#include "USART.h"

void App_Input_init(){

//	printf("App_Input_init\n");
	Keys_adv_init();
}
void App_Input_task(){ // 10ms
	
//	printf("Input task\n");
	
	uint8_t rst = Keys_adv_get();
	
	if(rst == 0x00) return;
	
	// 短按的4个按键数值为：0x01, 0x02, 0x03, 0x04
	// 长按的4个按键数值为：0xF1, 0xF2, 0xF3, 0xF4
	
	uint8_t key = rst & 0x0F;
	switch(key){
		case 0x01:
			printf("KEY1: %s\n", rst < 0xF0 ? "短按":"长按");
			break;
		case 0x02:
			printf("KEY2: %s\n", rst < 0xF0 ? "短按":"长按");
			break;
		case 0x03:
			printf("KEY3: %s\n", rst < 0xF0 ? "短按":"长按");
			USART2_send_string("AT\r\n");
			break;
		case 0x04:
			printf("KEY4: %s\n", rst < 0xF0 ? "短按":"长按");
			break;
		default:
			break;
	}
	
}