#include "tasks.h"


// 任务执行状态的判定和切换(在中断里1ms执行一轮)
void Task_switch_handler(void){
	// 1.不断给所有任务的count--
	// 2.判断count值，如果减到0，设置可执行标记
	// 3.把count重置为period
	for(uint8_t i = 0; i < task_cnt; i++){
		
		// 如果任务是挂起状态, 跳过本次循环
		if(task_list[i].state == TASK_STATE_SUSPEND){
			continue;
		}
		
		if(task_list[i].count > 0) task_list[i].count--;
		
		// 倒计数结束
		if(task_list[i].count == 0){
			// 任务状态切换
			task_list[i].state = TASK_STATE_RUN;
			// 重置count计数值
			task_list[i].count = task_list[i].period;
		}
	}
}

// 任务执行句柄（在main函数循环执行execute所有任务）
void Task_exec_handler(void){
	// 将状态为 TASK_STATE_RUN 执行一次，将标记恢复成STOP
	for(uint8_t i = 0; i < task_cnt; i++){
		if(task_list[i].state  == TASK_STATE_RUN){
			// 执行任务
			task_list[i].callback();
			// 任务状态切换
			task_list[i].state = TASK_STATE_STOP;
		}
	}
}
// 根据任务函数名，查找对应位置
int Task_get_task_id(Task_callback func_name){
	
	for(uint8_t i = 0; i < task_cnt; i++){
		if(task_list[i].callback == func_name){
			return i;
		}
	}
	
	return -1;
}

// 根据任务索引位置，获取任务状态
uint8_t Task_get_task_state(uint8_t index){
	if(index >= task_cnt) return TASK_STATE_STOP;
	
	return task_list[index].state;
}

// 根据任务索引位置，挂起指定任务（暂停）
void Task_suspend(uint8_t index){
	if(index >= task_cnt) return;
	
	// 设置为挂起状态
	task_list[index].state = TASK_STATE_SUSPEND;
	
	// 执行挂起回调函数（前提是此函数不是NULL）
	if(task_list[index].callback_suspend != NULL){
		task_list[index].callback_suspend();
	}
}

// 根据任务索引位置，恢复指定任务
void Task_resume(uint8_t index){
	if(index >= task_cnt) return;

	// 重置count计数值
	task_list[index].count = task_list[index].period;
	// 恢复状态
	task_list[index].state = TASK_STATE_STOP;
}


