# UART / YMODEM IAP

参考 IAP 模型通过 UART 接收 raw firmware，不依赖网络链路。Bootloader 解析 YMODEM packet，在 packet CRC 通过后写入内部 Flash 的 Application 区。这里描述的是本地 IAP transport，不是 Network OTA。

## 数据路径

```text
Host firmware file
        ↓ UART
YMODEM packet parser
        ↓
Sequence and CRC-16/XMODEM check
        ↓
Flash erase / program at 0x08004000
        ↓
Bootloader application handoff
```

## 协议要点

- `SOH`：128-byte data packet；
- `STX`：1024-byte data packet；
- packet number 与反码用于序号检查；
- `ACK` / `NAK` / `EOT` / cancel 控制传输；
- CRC-16/XMODEM 使用多项式 `0x1021`；
- 接收端解析文件大小，并在写入前与 Application 可用空间比较。

这里的 CRC 是 packet 传输错误检测。它不验证 firmware 来源；当前仓库也没有 whole-image cryptographic hash、signature 或 authenticated image header 实现。
- [Image Integrity](image-integrity.md)

[上一篇：Application Jump](application-jump.md) · [返回 README](../README.md) · [下一篇：OTA Control Plane](ota-control-plane.md)
