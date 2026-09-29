#include "bsp_keys_adv.h"
#include "systick.h"
#include <stdio.h>

typedef enum {
    KEY_RELEASE = 0, // 释放
    KEY_PRESSED,      // 按下
    KEY_SHORT_PRESS,  // 短按
    KEY_LONG_PRESS,   // 长按
} KEY_STATE;

typedef struct {
    rcu_periph_enum rcu;
    uint32_t port;
    uint32_t pin;
    KEY_STATE state;
    uint64_t prevSysTime;
} Key_GPIO_t;

static Key_GPIO_t g_gpio_list[] = {
    {RCU_GPIOC, GPIOC, GPIO_PIN_0},
    {RCU_GPIOC, GPIOC, GPIO_PIN_1},
    {RCU_GPIOC, GPIOC, GPIO_PIN_2},
    {RCU_GPIOC, GPIOC, GPIO_PIN_3},
};

uint8_t g_key_count = sizeof(g_gpio_list) / sizeof(Key_GPIO_t);

static void GPIO_init(rcu_periph_enum rcu, uint32_t port, uint32_t pin){
  rcu_periph_clock_enable(rcu);
  gpio_mode_set(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, pin);
}

void Keys_adv_init(void){
    for(uint8_t i = 0; i < g_key_count; i++){
        GPIO_init(g_gpio_list[i].rcu, g_gpio_list[i].port, g_gpio_list[i].pin);
    }
}

#define KEY_SHORT_PRESS_TIME  5     // 短按消抖
#define KEY_LONG_PRESS_TIME 600			// 600ms以上为长按

static uint8_t Keys_scan(uint8_t index){
    Key_GPIO_t* gpio = &g_gpio_list[index];

		// 获取按键是否按下
    uint8_t is_pressed = gpio_input_bit_get(gpio->port, gpio->pin) == RESET;

    switch(gpio->state){
        case KEY_RELEASE:		// 抬起状态
            if(is_pressed){
//								printf("key[%d] pressed\n", index);
                gpio->state = KEY_PRESSED;	
                gpio->prevSysTime = systick_get_tick();	// 记录按下时的时间戳
            }
            break;
        case KEY_PRESSED:		// 按下状态
            if(!is_pressed){
                gpio->state = KEY_RELEASE; // 消除抖动
                return 0;
            }
						// 还是按下
            if(systick_get_tick() - gpio->prevSysTime >= KEY_SHORT_PRESS_TIME){
                gpio->state = KEY_SHORT_PRESS;
            }
            break;
        case KEY_SHORT_PRESS:	// 判定为短按
            if(!is_pressed){
                // 短按抬起了
                gpio->state = KEY_RELEASE;
                return index + 1;           // 短按的4个按键数值为：0x01, 0x02, 0x03, 0x04
            }

            // 还没抬起，判定是否符合长按
            if(systick_get_tick() - gpio->prevSysTime >= KEY_LONG_PRESS_TIME){
                gpio->state = KEY_LONG_PRESS;
            }

            break;
        case KEY_LONG_PRESS: // 判定为长按
            if(!is_pressed){
                // 长按抬起了
                gpio->state = KEY_RELEASE;
                return index + 1 + 0xF0;        // 长按的4个按键数值为：0xF1, 0xF2, 0xF3, 0xF4
            }
            break;
        default:
            gpio->state = KEY_RELEASE;
            break;
    }
    return 0x00;
}

uint8_t Keys_adv_get(void){
    uint8_t rst = 0x00;
    for(uint8_t i = 0; i < g_key_count; i++){
        rst = Keys_scan(i);
        if(rst != 0x00){
            return rst;
        }
    }
	return 0x00;
}