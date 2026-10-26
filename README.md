# Embedded-OTA-Update-Lab

面向 Cortex-M 的 Bootloader、Flash layout、UART/YMODEM、MQTT transport 与 CRC 边界架构文档实验。

**🔄 Update Pipeline**

![Firmware update path](assets/images/architecture/portfolio-overview.svg)

## Update Snapshot

| Update Focus | Current Scope |
| --- | --- |
| Repository Type | Firmware Update Architecture Lab |
| Reference MCU | GD32F407VE / ARM Cortex-M4 |
| Local Path | UART / YMODEM IAP model |
| Network Path | MQTT block transport model |
| Integrity Boundary | CRC error detection；no authentication, encryption or rollback claim |
| Public Implementation | Host Flash byte-range planner and portable C block validator; no target firmware |
| Verification | Python range tests and C Host tests；target build, Flash-write and boot evidence not provided |

## 📌 Overview

仓库以原创文档和 SVG 描述 Bootloader / Application 边界、Flash 分区、向量跳转、UART/YMODEM IAP、MQTT 分块传输、更新元数据和 CRC 错误检测。

当前默认分支不分发课程 Bootloader、Flash/YMODEM/ESP8266/MQTT 应用代码、GD32/CMSIS 厂商组件、startup 或 Keil 工程。[Host 范围规划器](host_tools/flash_plan.py) 按本文参考分区计算整块与尾块地址；独立实现的 [C 分块校验核心](docs/portable-block-validator.md) 检查顺序、长度和块 CRC。两者都不执行 Flash 操作。Staging Backup 不是可启动 Slot B；文档不声明 A/B OTA、Automatic Rollback、Secure OTA、签名验证或生产级更新能力。

## 🏗️ Architecture

![Embedded OTA system stack](assets/images/architecture/ota-system-stack.svg)

```text
                  ┌─ UART / YMODEM ─┐
Firmware Source ──┤                  ├── Flash / Staging ── CRC ── Boot Handoff
                  └─ MQTT Transport ─┘
```

UART/YMODEM 与 MQTT 是独立 transport model。CRC 只表达传输或存储错误检测，不提供来源认证、保密性或防回滚能力。

## ✨ Key Features

| Capability | Documentation Entry |
| --- | --- |
| Boot/Application boundary | [Bootloader Architecture](docs/bootloader-architecture.md) |
| Vector handoff | [Application Jump](docs/application-jump.md) |
| Flash partition model | [Flash Layout](docs/flash-layout.md) |
| UART/YMODEM model | [YMODEM IAP](docs/ymodem-iap.md) |
| MQTT block model | [MQTT Block Transport](docs/mqtt-block-transport.md) |
| Integrity and failure boundary | [Image Integrity](docs/image-integrity.md) and [Failure Boundaries](docs/failure-boundaries.md) |
| Portable receive validation | [Block Validator](docs/portable-block-validator.md) |

## 📂 Project Structure

```text
Embedded-OTA-Update-Lab/
├── README.md
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── assets/images/  # Repository-authored architecture and flow SVG
├── docs/           # Firmware-update architecture documentation
├── host_tools/     # Host-only byte-range model
├── include/, src/  # Portable C block validator; no hardware adapter
└── tests/          # Python and C Host tests
```

## 📚 Documentation

- [Bootloader Architecture](docs/bootloader-architecture.md)
- [Flash Layout](docs/flash-layout.md)
- [Application Jump](docs/application-jump.md)
- [YMODEM IAP](docs/ymodem-iap.md)
- [OTA Control Plane](docs/ota-control-plane.md)
- [MQTT Block Transport](docs/mqtt-block-transport.md)
- [Image Integrity](docs/image-integrity.md)
- [Failure Boundaries](docs/failure-boundaries.md)
- [Development Environment](docs/development-environment.md)
- [Portable Block Validator](docs/portable-block-validator.md)

## 🧪 Verification

| Verification Layer | Status | Boundary |
| --- | --- | --- |
| Host Test | 7 Python tests plus C validator tests | Range planning, contiguous receive, block CRC and invalid input/state boundaries |
| Target Build Verification | NOT PROVIDED | No Keil project or target firmware source is distributed |
| Hardware Validation | NOT PROVIDED | No reviewable Flash, UART, network, or boot result |
| Runtime Evidence | NOT PROVIDED | No transfer log, Flash trace, MQTT session, or boot record |

在 Python 3.10+ 环境运行 `python -m unittest discover -s tests -v`。示例 `python -m host_tools.flash_plan 257` 打印从 Staging 到 Active 的计划字节范围。C 模块的 GCC/Clang 构建命令与最小调用示例见 [Portable Block Validator](docs/portable-block-validator.md)。Python 规划器不校验 CRC；C 校验器不操作 Flash 或连接设备。Host 测试不能证明设备端擦写、YMODEM/MQTT 兼容、整包认证、掉电恢复或升级成功。

## License Boundary

根目录 MIT License 仅覆盖当前默认分支中仓库维护者编写的文档、配置、自绘 SVG、Host 范围规划器、C 分块校验核心及测试。课程源码、GD32/CMSIS、startup、Keil 工程和第三方 transport 实现未包含在当前默认分支；旧提交中的历史文件仍需单独评估。详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
