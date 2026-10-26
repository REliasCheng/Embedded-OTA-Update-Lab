# Development Environment

## Platform

- MCU: GD32F407VE
- Core: ARM Cortex-M4
- Internal Flash: 512 KiB
- Device pack: GigaDevice GD32F4xx DFP 3.2.0
- Firmware support: GD32F4xx Standard Peripheral Library and CMSIS

## Project Boundary

当前默认分支包含原创架构文档、自绘 SVG、Python Host 范围规划器、独立 C11 校验核心、Flash 接口及 Host 内存模拟器，不包含 Keil `.uvprojx`、CMSIS、vendor library、startup、Bootloader/Application 目标工程、构建产物或固件二进制。

## Build status

当前分支有可在 Host 使用 GCC/Clang 构建的独立 C 模块，但没有目标固件工程，因此：

```text
HOST_C_BUILD=AVAILABLE
TARGET_BUILD_NOT_AVAILABLE=YES
```

历史 Keil log、工程文件或源码存在性不能替代当前编译或硬件验证。

## Runtime status

当前分支没有烧录、UART/YMODEM 传输、Wi-Fi/MQTT 连接、OTA 或 Flash update 运行证据。

[上一篇：Failure Boundaries](failure-boundaries.md) · [返回 README](../README.md)
