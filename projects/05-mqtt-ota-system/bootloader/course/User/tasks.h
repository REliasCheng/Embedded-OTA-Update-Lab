#ifndef __TASKS_H__
#define __TASKS_H__

#include "gd32f4xx.h"

#define TASK_STATE_STOP				0	// 任务停止
#define TASK_STATE_RUN				1 // 可以运行
#define TASK_STATE_SUSPEND		2 // 任务挂起（暂停）

typedef void (* Task_callback)(void);

// 使用结构体数组，封装所有的任务
typedef struct Task{
	
	uint8_t 		state;		// 任务状态，2挂起 1可执行，0等待中
	uint16_t 		count;		// 任务计数，倒计数到0，修改任务状态为可执行
	uint16_t   period;    // 任务周期，即多少ms执行一次任务（固定）
	
//	void (* callback)(void); // 任务执行回调
	Task_callback		callback;	// 任务执行回调
	Task_callback		callback_suspend;	// 任务执行回调
	
} Task_t;

extern Task_t task_list[];
extern uint8_t task_cnt;

extern void Task_init(void);

// 任务执行状态的判定和切换(1ms执行一轮)
void Task_switch_handler(void);

// 任务执行句柄（循环执行execute所有任务）
void Task_exec_handler(void);

// 挂起指定任务（暂停）
void Task_suspend(uint8_t index);

// 恢复指定任务
void Task_resume(uint8_t index);

// 根据任务函数名，查找对应位置
int Task_get_task_id(Task_callback func_name);

// 根据任务索引位置，获取任务状态
uint8_t Task_get_task_state(uint8_t index);

#endif