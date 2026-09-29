#include "tasks.h"
#include "App.h"

void Task_init(void){
	
	App_Input_init();
	App_OTA_init();
	App_Wifi_init();
}

// 所有任务结构体列表
Task_t task_list[] = {
		// state		count		period 		callback
		{TASK_STATE_STOP, 0,	 10,		App_Input_task},	// 输入事件
		{TASK_STATE_STOP, 0,	200,		App_OTA_task},		// 屏幕刷新任务
		{TASK_STATE_STOP, 0,	 10,		App_Wifi_task},	// 输入事件
};
uint8_t task_cnt = sizeof(task_list)/sizeof(Task_t);