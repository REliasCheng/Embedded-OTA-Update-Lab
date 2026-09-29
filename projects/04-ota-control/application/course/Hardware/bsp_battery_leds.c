#include "bsp_battery_leds.h"
#include "systick.h"
// 声明GPIO初始化所需参数的结构体
typedef struct {
  rcu_periph_enum rcu;
  uint32_t port;
  uint32_t pin;
} Led_GPIO_t;

// 声明所有GPIO的参数的数组 global
Led_GPIO_t g_gpio_list[] = {
  {RCU_GPIOC, GPIOC, GPIO_PIN_6 },   // LED_SW
  {RCU_GPIOD, GPIOD, GPIO_PIN_8 },   // LED1
  {RCU_GPIOD, GPIOD, GPIO_PIN_9 },   // LED2
  {RCU_GPIOD, GPIOD, GPIO_PIN_10},   // LED3
  {RCU_GPIOD, GPIOD, GPIO_PIN_11},   // LED4
};

// 数组长度
#define MAX_LED_COUNT   (sizeof(g_gpio_list) / sizeof(Led_GPIO_t))

static void GPIO_config(rcu_periph_enum rcu, uint32_t port, uint32_t pin) {

  // 1. 时钟初始化 ------------------------ PC6
  rcu_periph_clock_enable(rcu);
  // 2. 配置GPIO输入输出模式
  gpio_mode_set(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
  // 3. 配置GPIO输出模式的选项options
  gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, pin);

}


void Battery_Leds_init() {
  uint8_t count = MAX_LED_COUNT;
  for(uint8_t i = 0; i < count; i++) {
    Led_GPIO_t gpio = g_gpio_list[i];
    // 初始化每个gpio
    GPIO_config(gpio.rcu, gpio.port, gpio.pin);
    // 默认关闭（拉高）
    gpio_bit_set(gpio.port, gpio.pin);
  }

  // 总开关拉低打开
  gpio_bit_reset(g_gpio_list[0].port, g_gpio_list[0].pin);
}

// 点亮某个灯
void Battery_Leds_turn_on(uint8_t index) {
  if(index >= MAX_LED_COUNT) return;  // 避免索引越界

  Led_GPIO_t gpio = g_gpio_list[index];
  gpio_bit_reset(gpio.port, gpio.pin);
}

// 熄灭某个灯
void Battery_Leds_turn_off(uint8_t index) {
  if(index >= MAX_LED_COUNT) return; // 避免索引越界

  Led_GPIO_t gpio = g_gpio_list[index];
  gpio_bit_set(gpio.port, gpio.pin);
}

void Battery_Leds_turn(uint8_t index, uint8_t state) {
  if(index >= MAX_LED_COUNT) return; // 避免索引越界
  Led_GPIO_t gpio = g_gpio_list[index];
  gpio_bit_write(gpio.port, gpio.pin, state ? RESET : SET);
}

// 业务相关函数==============================================================

// 开始充电 // [0,4] 500毫秒更新一次电量
// *,-,-,-
// *,*,-,-
// *,*,*,-
// *,*,*,*
// *,-,-,-
// *,*,-,-

uint8_t state = 0;  // 0.停止  1.充电中, 2.停止中
uint8_t show_power = 0;
uint8_t current_power = 0;
void Battery_Leds_start(uint8_t power) {
  current_power = power;
  show_power = current_power;
  state = 1; // 修改为充电中状态
}
// 通过状态机，在不同函数里修改状态
void Batter_Leds_loop() {
  if (state == 0) {    // 停止

    Battery_Leds_turn_off(LED1);
    Battery_Leds_turn_off(LED2);
    Battery_Leds_turn_off(LED3);
    Battery_Leds_turn_off(LED4);

  } else if(state == 1) { // 充电中
    Battery_Leds_turn(LED1, show_power >= 1);
    Battery_Leds_turn(LED2, show_power >= 2);
    Battery_Leds_turn(LED3, show_power >= 3);
    Battery_Leds_turn(LED4, show_power >= 4);

    // 重置电量为当前电量
    if(++show_power > 4) show_power = current_power;
  } else if(state == 2) { // 停止中
    // 当前电量闪三次
    int cnt = 3;
    while(cnt--) {
      Battery_Leds_turn_off(LED1);
      Battery_Leds_turn_off(LED2);
      Battery_Leds_turn_off(LED3);
      Battery_Leds_turn_off(LED4);
      delay_1ms(200);
      Battery_Leds_turn(LED1, current_power >= 1);
      Battery_Leds_turn(LED2, current_power >= 2);
      Battery_Leds_turn(LED3, current_power >= 3);
      Battery_Leds_turn(LED4, current_power >= 4);
      delay_1ms(200);
    }
    state = 0; // 真正停止
  }

}

// 结束充电
void Battery_Leds_stop() {
  state = 2;
}

// 更新电量
void Battery_Leds_update(uint8_t power) {
  current_power = power;
}

