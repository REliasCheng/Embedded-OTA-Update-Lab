# Image Integrity

架构模型区分三类 CRC，用于不同层级的数据错误检测。它们不验证 firmware 来源，也不构成 firmware authentication。

| Mechanism | Scope | Polynomial / form | Engineering role |
|---|---|---|---|
| CRC-16/XMODEM | YMODEM packet | `0x1021` | UART packet error detection |
| CRC-16/IBM | OTA firmware block | `0xA001` | MQTT block payload error detection |
| CRC8 | Parameter record | implementation-defined in project | Flash parameter record consistency |

当前公开实现的 [C 分块校验器](portable-block-validator.md) 使用反射多项式 `0xA001`，初值与最终 XOR 由调用者显式给出；已用 CRC-16/ARC 与 CRC-16/MODBUS 标准检查值进行 Host 测试。它只校验传入的单块字节，不解析 MQTT、YMODEM 或设备 CRC 字段，因此不能推断与资料中的网络协议直接兼容。YMODEM 的 CRC-16/XMODEM 不作为此模块的默认算法。

独立的 [映像元数据核心](image-manifest.md)使用 CRC-32/ISO-HDLC 检查完整映像，并在 Host 测试中核对公开检查值 `123456789 → 0xCBF43926`。分块接收完成与整体映像校验成功是两个不同结果；CRC32 仍不认证来源。

## Cryptographic Boundary

- Whole-image hash；
- Digital signature / public-key verification；
- Encrypted firmware；
- Secure Boot / Root of Trust；
- Authenticated image manifest；
- Anti-rollback version counter。

CRC 可以发现部分传输或存储错误，但不能证明映像来源，也不能抵抗恶意篡改。因此仓库不使用 `Secure OTA`、`Authenticated Firmware` 或 `Verified Boot` 描述当前架构。

[上一篇：MQTT Block Transport](mqtt-block-transport.md) · [返回 README](../README.md) · [下一篇：Failure Boundaries](failure-boundaries.md)
