# Bootloader Architecture

参考模型将 Bootloader 放在 `0x08000000`，负责更新条件判断、固件接收或复制，以及向固定 Application 地址交接执行权。它不承担常规应用逻辑；当前默认分支不包含实现。

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

当前仓库没有可审查实现来证明 MSP/Reset Handler 检查、image header、whole-image hash、数字签名或 boot confirmation。架构也不包含两个可启动 Slot。

[返回 README](../README.md) · [下一篇：Flash Layout](flash-layout.md)
