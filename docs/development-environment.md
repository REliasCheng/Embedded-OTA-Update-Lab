# Development Environment

## Platform

- MCU: GD32F407VE
- Core: ARM Cortex-M4
- Internal Flash: 512 KiB
- Device pack: GigaDevice GD32F4xx DFP 3.2.0
- Firmware support: GD32F4xx Standard Peripheral Library and CMSIS

## Project Boundary

当前默认分支只包含原创架构文档与自绘 SVG，不包含 Keil `.uvprojx`、CMSIS、vendor library、startup、Bootloader/Application 源码、构建产物或固件二进制。

## Build status

当前分支没有可构建工程，因此：

```text
BUILD_PASS=0
BUILD_FAIL=0
BUILD_NOT_AVAILABLE=YES
```

历史 Keil log、工程文件或源码存在性不能替代当前编译或硬件验证。

## Runtime status

当前分支没有烧录、UART/YMODEM 传输、Wi-Fi/MQTT 连接、OTA 或 Flash update 运行证据。

[上一篇：Failure Boundaries](failure-boundaries.md) · [返回 README](../README.md)
