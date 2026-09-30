# Application Jump

Bootloader 与 Application 的交接依赖 Cortex-M vector table 的前两个 word：初始 MSP 和 Reset Handler。

## Bootloader 侧

最终 OTA Bootloader 的跳转顺序为：

1. 从固定 Application 起点读取初始 MSP；
2. 检查 MSP 是否位于 `0x20000000`–`0x2001FFFF`；
3. 关闭 USART0、USART2、SysTick、TIMER6 等当前使用的 IRQ，并关闭全局中断；
4. 通过 `__set_MSP()` 写入 Application 初始栈指针；
5. 从 `ApplicationStart + 4` 读取 Reset Handler；
6. 以函数指针进入 Application reset path。

不同工程族使用不同固定地址：

- Boot/App split 与 YMODEM IAP pair：Application `0x08004000`，vector offset `0x4000`；
- MQTT OTA pair：Application `0x08008000`，vector offset `0x8000`。

地址来自各自的 link configuration 与启动代码，不能用同一个 `APPLICATION_START` 描述全部配对工程。

## Application 侧

Application 的早期初始化调用 `nvic_vector_table_set(NVIC_VECTTAB_FLASH, BOOTLOADER_SIZE)`：

- 基础/IAP：`BOOTLOADER_SIZE = 0x4000`；
- OTA：`BOOTLOADER_SIZE = 0x8000`。

向量表重定位由 Application 完成，不是 Bootloader 跳转函数直接写 `VTOR`。

## Handoff 边界

现有跳转代码没有完整验证 Reset Handler 地址范围与 Thumb state，也没有统一清理全部 NVIC enable/pending 状态和所有外设。这里描述的是当前 handoff sequence，不代表已经完成通用、安全的启动交接验证。

## 工程入口

- [Bootloader / Application Split](../projects/02-boot-app-split/)
- [UART / YMODEM IAP](../projects/03-ymodem-iap/)
- [MQTT OTA System](../projects/05-mqtt-ota-system/)

[上一篇：Flash Layout](flash-layout.md) · [返回 README](../README.md) · [下一篇：YMODEM IAP](ymodem-iap.md)
