# Development Environment

## Reference platform

- MCU: GD32F407VE / ARM Cortex-M4
- Internal Flash: 512 KiB (reference capacity; the published partition model has not been checked against target erase sectors)
- Historical project references: GigaDevice GD32F4xx DFP 3.2.0, GD32F4xx Standard Peripheral Library and CMSIS. These components are **not** bundled with the current default branch or verified as an installed, usable target build environment.

## 1. Host software — verified

当前默认分支包含原创 C11 分块与映像校验核心、可注入 Flash 接口、内存 Flash 模拟器、[Host OTA 更新状态机](host-update-flow.md)、Python 地址范围规划器及测试。普通 PC 上已运行：

- Python 7 项测试。
- C 44 项测试：分块校验 14 项、映像校验 8 项、Flash 模拟 6 项、OTA 集成 16 项。
- GCC、Clang 构建与运行，以及 ASan、UBSan CI。
- `normal`、`corrupt`、`recovery` 三条可执行 Host Demo 路径。

从仓库根目录运行 Python 测试：

```sh
python -m unittest discover -s tests -v
```

C Demo 的完整 GCC 构建与三条运行命令见 [Host Update Flow](host-update-flow.md)。这些结果验证模拟 Staging、CRC32、复制、Journal 和故障路径；它们不是设备端 OTA 运行结果。

## 2. GD32 目标工程 — 未验证

当前默认分支没有公开可复现的 GD32F407VE Bootloader/Application 目标工程、Keil `.uvprojx`、启动文件、链接配置、厂商组件或目标固件二进制。尚未取得目标工具链可用性、芯片 Flash 擦除几何、Application 链接边界与向量交接的完整验证证据。因此没有目标编译、链接、内存报告或目标固件运行结论。历史 Keil 日志不能替代当前源码的目标构建。

```text
HOST_SOFTWARE_VERIFIED=YES
TARGET_BUILD_VERIFIED=NO
```

## 3. 真实设备 — 未授权、未验证

本项目没有执行真实 Flash 擦写、固件升级、UART/YMODEM 或 MQTT 设备传输、板端跳转与掉电故障注入；没有相应的实机日志。Host 模拟器的恢复测试不能证明设备擦写安全、原子更新或物理启动成功。

```text
HARDWARE_VERIFIED=NO
```

[上一篇：Failure Boundaries](failure-boundaries.md) · [返回 README](../README.md)
