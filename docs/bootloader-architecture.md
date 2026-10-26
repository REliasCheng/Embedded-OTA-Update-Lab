# Bootloader Architecture

参考模型将 Bootloader 放在 `0x08000000`，负责更新条件判断、固件接收或复制，以及向固定 Application 地址交接执行权。它不承担常规应用逻辑。当前默认分支实现了独立的 Host OTA 软件模型，**不包含 GD32 Bootloader 固件或实际应用跳转**。

## 启动与更新路径

```text
Reset
  ↓
Bootloader initialization
  ↓
Update condition?
  ├─ No  → validate initial MSP → jump to Application
  └─ Yes → receive/copy firmware → update metadata → Application handoff
```

文档分别展示 16 KiB Bootloader / `0x08004000` Application 和 32 KiB Bootloader / `0x08008000` Application 两种参考布局。地址必须与配对 Application 的 link configuration 和 `VTOR` offset 一致。

YMODEM IAP 与 MQTT OTA 使用不同 transport：前者从 UART packet 直接写入 Application 区，后者在 Bootloader 中请求网络固件块并写入 Staging Backup。二者共享 Flash programming 与 application handoff 基础，但不是同一数据路径。

## OTA Model

- Application 查询版本元数据并写入更新意图；
- 复位后 Bootloader 检查更新标志；
- Bootloader 通过网络传输链路获取 firmware block；
- block 经 CRC 校验后写入 Staging Backup；
- 下载路径结束后，Bootloader 将 staging 内容复制到固定 Active App 区；
- 后续启动仍从 `0x08008000` 读取 Application vector table。

## 验证边界

当前已有 Host 端 `OtaImageManifest`，明确记录格式版本、固件版本、映像长度和 CRC32；[映像元数据核心](image-manifest.md)及[Host 更新流程](host-update-flow.md)会校验整包数据。`ota_update_boot_eligible()` 只是模拟状态为 `APPLIED` 且 Active 整包 CRC32 读回一致时的 Host 启动资格判断。

尚无真实 Cortex-M Reset Handler 跳转，也没有 MSP、Reset Handler 地址和向量表合法性的目标端验证。CRC32 是意外损坏检测值，**不是密码学意义的 whole-image hash**，不能提供数字签名、可信启动或实际目标固件认证。参考分区中的单 Active 加 Staging 也不是两个可启动的 A/B Slot；Host 状态机不等于已运行的 GD32 Bootloader。

[返回 README](../README.md) · [下一篇：Flash Layout](flash-layout.md)
