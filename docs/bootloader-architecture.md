# Bootloader Architecture

Bootloader 位于 `0x08000000`，负责更新条件判断、固件接收或复制，以及向固定 Application 地址交接执行权。它不承担常规应用逻辑。

## 两类启动路径

```text
Reset
  ↓
Bootloader initialization
  ↓
Update condition?
  ├─ No  → validate initial MSP → jump to Application
  └─ Yes → receive/copy firmware → update metadata → Application handoff
```

基础与 IAP 系统把 Application 放在 `0x08004000`；最终 OTA 系统将 Bootloader 扩展到 32 KiB，Application 改为 `0x08008000`。地址必须与配对 Application 的 Keil link configuration 和 `VTOR` offset 一致。

## 最终 OTA 分工

- Application 查询版本元数据并写入更新意图；
- 复位后 Bootloader 检查更新标志；
- Bootloader 通过网络传输链路获取 firmware block；
- block 经 CRC 校验后写入 Staging Backup；
- 下载路径结束后，Bootloader 将 staging 内容复制到固定 Active App 区；
- 后续启动仍从 `0x08008000` 读取 Application vector table。

## 当前验证边界

跳转路径检查初始 MSP 是否位于 SRAM1 范围，但没有形成完整 image header、整包 hash、数字签名或 boot confirmation。Bootloader 也没有在两个可启动 Slot 之间选择。

## 工程入口

- [Bootloader / Application Split](../projects/02-boot-app-split/)
- [UART / YMODEM IAP](../projects/03-ymodem-iap/)
- [MQTT OTA System](../projects/05-mqtt-ota-system/)

[返回 README](../README.md) · [下一篇：Flash Layout](flash-layout.md)
