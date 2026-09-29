# UART / YMODEM IAP

## 项目作用

通过 UART 接收 YMODEM firmware packet，将 raw image 写入 `0x08004000` Application 区，再执行固定地址启动。

## 系统配对

```text
Host
  ↓ UART / YMODEM
Bootloader → CRC-16/XMODEM → Internal Flash
  ↓
Application at 0x08004000
```

## 技术内容

- SOH 128-byte 与 STX 1024-byte packet；
- packet sequence、ACK/NAK/EOT；
- CRC-16/XMODEM packet 校验；
- Flash 擦写与 Bootloader/Application 交接。

## 工程入口

- [IAP Bootloader](bootloader/course/)
- [Paired Application](application/course/)
- [YMODEM IAP mechanism](../../docs/ymodem-iap.md)

[上一项：Boot/App Split](../02-boot-app-split/) · [返回工程索引](../README.md) · [下一项：OTA Control](../04-ota-control/)
