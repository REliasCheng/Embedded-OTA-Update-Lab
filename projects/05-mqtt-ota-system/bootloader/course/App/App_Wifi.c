#include "App.h"
#include "USART.h"
#include "wifi_drv.h"
#include "systick.h"

void App_Wifi_init(){
//	printf("App_Wifi_init\n");
	WifiModuleDrvInit();
}
void App_Wifi_task(){

}