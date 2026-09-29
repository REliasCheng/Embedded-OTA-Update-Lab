#include "App.h"
#include "oled.h"
#include "store_app.h"
#include "update.h"
#include "tasks_timer.h"
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
	
	InitSysParam();
//	printf("App_OTA_init\n");
	OLED_Init();
}

bool is_update_enable = false;

char cur_version[10] = "";
char new_version[10] = "";
static uint64_t tick = 0;
#define TASK_INTERVAL		10

void App_OTA_task(){
//	printf("App_OTA_task\n");
	INTERVAL_CHECK(tick, TASK_INTERVAL); // 10ms
	
	uint32_t bin_size;
	is_update_enable = false;
	
	char strbuf[30] = "";
	uint8_t y = 0;
	
	OLED_Clear(0); // 只清空缓存，不显示到屏幕
	
	// 当前版本号
	GetSoftwareVersionParam(cur_version);
	sprintf(strbuf, "Cur Ver: %s\n", cur_version);
	OLED_ShowString(0, y, strbuf, 8, 1);
	
	// 检查最新版本号
	if(!GetNewVersionInfo(new_version, &bin_size)){
		OLED_Refresh();
		return;
	}
	is_update_enable = true;
	
	// 拿到了新版本
	sprintf(strbuf, "New Ver: %s\n", new_version);
	OLED_ShowString(0, y+=8, strbuf, 8, 1);
	
	// 显示新固件大小
	sprintf(strbuf, "Bin Size: %d\n", bin_size);
	OLED_ShowString(0, y+=8, strbuf, 8, 1);
	
	// 显示下载百分比
	uint8_t progress = GetUpdateProgress();
	if(progress > 100) progress = 100;
	
	sprintf(strbuf, "->Downloading: %d%%\n", progress);
	OLED_ShowString(0, y+=16, strbuf, 8, 1);
	
	if(progress == 100){
		OLED_ShowString(0, y+=8, "->Writing Bin.", 8, 1);
	}
	
	OLED_Refresh();
}
#include "iap_driver.h"

void App_OTA_update(){
	if(!is_update_enable){
		printf("未发现新版本,不能升级\n");
		return;
	}
	
	printf("进行重启升级，当前版本:[%s]，新版本:[%s]\n", cur_version, new_version);
	// 设置升级标记
	SetUpdateVerFlag();
	
	// 重启到Bootloader
	iap_reset_to_boot();
}




