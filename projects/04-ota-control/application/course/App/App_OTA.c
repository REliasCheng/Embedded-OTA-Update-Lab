#include "App.h"
#include "oled.h"


/*****************
	1. 展示当前版本信息
	2. 展示最新版本号
	3. 展示最新的固件大小
	4. 显示长按确认重启升级

解决闪屏：
1. 使用缓存，擦除缓存，合并所有内容后，一次性刷新
2. 先擦除原内容，显示新的内容
********************/

void App_OTA_init(){
//	printf("App_OTA_init\n");
	OLED_Init();
}

void App_OTA_task(){
//	printf("App_OTA_task\n");
	char version[10] = "1.0";
	
	char strbuf[30] = "";
	uint8_t y = 0;
	
	OLED_Clear(0); // 只清空缓存，不显示到屏幕
	
	OLED_ShowString(0, y, "OTA check...", 16, 0);
	// 当前版本号
	sprintf(strbuf, "Cur Ver: %s\n", version);
	OLED_ShowString(0, y+=16, strbuf, 16, 1);
	
	OLED_Refresh();
}