#include "App.h"
#include "bsp_keys_adv.h"
#include "USART.h"
#include "wifi_drv.h"
#include "iap_driver.h"
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
	
	WifiCommState_t commState;
	
	uint8_t key = rst & 0x0F;
	switch(key){
		case 0x01:
			printf("KEY1: %s\n", rst < 0xF0 ? "短按":"长按");
			if(rst >= 0xF0){
				App_OTA_update();
			}
		
			break;
		case 0x02:
			printf("KEY2: %s\n", rst < 0xF0 ? "短按":"长按");
			
			commState = AtCmdHandle("AT\r\n", "OK\r\n", 200);
			printf("commState: %d\n", commState);
		break;
		case 0x03:
			printf("KEY3: %s\n", rst < 0xF0 ? "短按":"长按");
			SendWifiModuleStr("AT+RST\r\n");
			
			break;
		case 0x04:
			printf("KEY4: %s\n", rst < 0xF0 ? "短按":"长按");
			SendWifiModuleStr("AT+GMR\r\n");
			break;
		default:
			break;
	}
	
}