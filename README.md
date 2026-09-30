# 嵌入式 OTA 升级实验室
## Embedded OTA Update Lab

基于 GD32F407VE / ARM Cortex-M4，围绕 Bootloader、Application Offset、UART/YMODEM IAP、MQTT 固件分块传输、Flash 更新与 CRC 检查组织的嵌入式固件升级仓库。

![Embedded OTA system stack](assets/images/architecture/ota-system-stack.svg)

**Platform:** GD32F407VE / Cortex-M4 · **Bootloader:** `0x08000000` · **Applications:** `0x08004000` / `0x08008000` · **Systems:** 5 · **Keil Projects:** 8

## 👋 项目简介 | Overview

仓库沿 Flash programming、Bootloader/Application split、UART/YMODEM IAP、OTA control plane 和 MQTT block transport 组织 5 条递进主线。Bootloader 与 Application 按系统配对保存，便于同时核对 link address、vector-table offset、传输协议和启动交接。

IAP 描述设备内部的固件接收与写入机制；Network OTA 在此基础上增加远程元数据、分块传输和更新状态。两者不是同义词，网络连接本身也不等于完整 OTA 系统。

## ⚙ 技术范围 | Technical Scope

- **Flash Foundation**：内部 Flash 擦除、写入、读取与分区边界。
- **Boot Handoff**：初始 MSP、Reset Vector、固定地址跳转与 Application VTOR relocation。
- **UART / YMODEM IAP**：128/1024-byte packet、序号、ACK/NAK/EOT 与 CRC-16/XMODEM。
- **OTA Control Plane**：版本元数据、更新意图、更新标志与复位交接。
- **MQTT Block Transport**：ESP8266 类 AT 模组、Aliyun IoT OTA topic、分块请求与 CRC-16/IBM。
- **Image State**：Active App、Staging Backup、Update Info 与 Parameters。

## 🧠 Bootloader 架构 | Bootloader Architecture

```text
Reset
  ↓
Bootloader at 0x08000000
  ├── no update → validate initial MSP → fixed Application handoff
  └── update    → receive/copy image → update state → Application handoff
```

基础与 YMODEM IAP 系统使用 `0x08004000` Application；最终 MQTT OTA 系统使用 `0x08008000` Application。配对工程的 link address 与 `VTOR` offset 必须一致。

- [Bootloader Architecture](docs/bootloader-architecture.md)
- [Application Jump](docs/application-jump.md)

## 🗺 Flash 布局 | Flash Layout

![OTA flash memory map](assets/images/diagram/flash-memory-map.svg)

最终 OTA 布局由 32 KiB Bootloader、237 KiB Active App、237 KiB Staging Backup、2 KiB Update Info 和 4 KiB Parameters 组成。Staging Backup 是下载暂存与复制来源，不是可启动 Slot B。基础/IAP 系统的 `0x08004000` Application 布局单独记录在 [Flash Layout](docs/flash-layout.md) 中。

## 📦 IAP 与固件传输 | IAP & Firmware Transport

```text
Local IAP:   Host File → UART / YMODEM → Packet CRC → Flash at 0x08004000
Network OTA: Aliyun Metadata / Blocks → MQTT / AT → Block CRC → Staging Flash
```

YMODEM 负责本地 UART firmware transport；最终 OTA 系统通过 USART2 控制 ESP8266 类 AT 模组，并使用 Aliyun MQTT topic 取得元数据和固件块。两条路径最终都依赖 Bootloader 与内部 Flash 更新机制。

- [UART / YMODEM IAP](docs/ymodem-iap.md)
- [OTA Control Plane](docs/ota-control-plane.md)
- [MQTT Block Transport](docs/mqtt-block-transport.md)

## 📡 OTA 数据通路 | OTA Data Flow

![Firmware update flow](assets/images/diagram/firmware-update-flow.svg)

最终 OTA Application 处理版本元数据和更新意图；复位后，Bootloader 请求固件块、执行 CRC-16/IBM 检查、写入 Staging Backup，并将镜像复制到固定 Active App 区。CRC 用于错误检测，不提供数字签名或来源认证。

## 🚀 核心系统 | Featured Systems

### [Flash Programming Foundation](projects/01-flash-basics/)

GD32F407 内部 Flash 的 sector-aware erase、read 与 write 基础。

### [Bootloader / Application Split](projects/02-boot-app-split/)

Bootloader 与 `0x08004000` Application 配对，包含 MSP/Reset Vector 跳转和 `VTOR` offset。

### [UART / YMODEM IAP](projects/03-ymodem-iap/)

通过 UART/YMODEM 接收 raw firmware，以 CRC-16/XMODEM 检查 packet 后写入 Application 区。

### [OTA Control Plane](projects/04-ota-control/)

Application 侧的版本元数据、更新确认、更新标志和复位交接。

### [MQTT OTA System](projects/05-mqtt-ota-system/)

Application/Bootloader 配对系统：Aliyun MQTT 固件分块传输、Staging Flash、Active App copy 与固定地址启动。

[查看完整工程索引](projects/)

## 📂 仓库结构 | Repository Structure

```text
Embedded-OTA-Update-Lab/
├── assets/images/           # OTA stack、Flash Map 与更新流程图
├── docs/                    # Bootloader、IAP、Transport、CRC 与实现边界
├── projects/                # 5 条主线系统；Bootloader/Application 成对组织
├── SOURCE_SELECTION_MANIFEST.csv
└── MIGRATION_HASH_VERIFICATION.csv
```

## 🛠 开发环境 | Development Environment

- GD32F407VE / ARM Cortex-M4
- Keil MDK-ARM / ArmClang project definitions
- GigaDevice GD32F4xx DFP 3.2.0
- GD32F4xx Standard Peripheral Library / CMSIS
- ESP8266-class AT Wi-Fi module in the network OTA path

当前机器未发现可直接调用的 Keil `UV4.exe`，8 个 Keil 工程均未在本阶段自动构建。历史构建日志未纳入公开仓库，也不作为本轮构建通过证据。详见 [Development Environment](docs/development-environment.md)。

## 📖 技术文档 | Documentation

- [Bootloader Architecture](docs/bootloader-architecture.md)
- [Flash Layout](docs/flash-layout.md)
- [Application Jump](docs/application-jump.md)
- [UART / YMODEM IAP](docs/ymodem-iap.md)
- [OTA Control Plane](docs/ota-control-plane.md)
- [MQTT Block Transport](docs/mqtt-block-transport.md)
- [Image Integrity](docs/image-integrity.md)
- [Failure Boundaries](docs/failure-boundaries.md)
- [Development Environment](docs/development-environment.md)

## 📜 来源与许可 | License

公开快照排除了构建产物、历史日志、IDE 用户状态、固件二进制和许可不明视觉资源。最终 OTA 工程中的网络与云端凭据已在公开派生副本中替换为占位符；变更范围记录在两个迁移 Manifest 中。

仓库新增 Markdown 与自有 SVG 适用根目录 [LICENSE](LICENSE)。迁移源码保留原有版权头并继续受各自条款约束，详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
