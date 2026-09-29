# Development Environment

## Platform

- MCU: GD32F407VE
- Core: ARM Cortex-M4
- Internal Flash: 512 KiB
- Device pack: GigaDevice GD32F4xx DFP 3.2.0
- Firmware support: GD32F4xx Standard Peripheral Library and CMSIS

## Project organization

公开仓库包含 5 条主线系统和 8 个 Keil `.uvprojx`：Flash foundation 1 个、Boot/App split 2 个、YMODEM IAP 2 个、OTA control 1 个、最终 MQTT OTA pair 2 个。

项目文件保留原始相对目录结构；`Objects`、`Listings`、固件二进制、历史日志和 IDE 用户状态未迁移。

## Build status

当前机器未发现可直接调用的 Keil `UV4.exe`，因此 8 个 Keil 工程均未在本阶段自动构建：

```text
BUILD_PASS=0
BUILD_FAIL=0
BUILD_NOT_AUTOMATED=8
```

源资料中的历史 Keil log 未进入公开仓库，也不计作本轮 build result。工程文件完整性和源码存在性不能替代编译或硬件验证。

## Runtime status

本阶段未执行烧录、UART/YMODEM 传输、Wi-Fi/MQTT 连接、Aliyun OTA 或 Flash update。源码中的 log string 不是运行证据。

[上一篇：Failure Boundaries](failure-boundaries.md) · [返回 README](../README.md)
