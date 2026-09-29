#include "App.h"

void USART2_on_recv(uint8_t* data, uint32_t len){
	printf("USART2_[%d]: %s\n", len, data);
	
	if(strstr((const char *)data, "OK") != NULL){
		printf("œÏ”¶≥…π¶\n");
	}
}

void App_Wifi_init(){
//	printf("App_Wifi_init\n");
}

void App_Wifi_task(){
//	printf("App_Wifi_task\n");
	
	
}