# Image Integrity

架构模型区分三类 CRC，用于不同层级的数据错误检测。它们不验证 firmware 来源，也不构成 firmware authentication。

| Mechanism | Scope | Polynomial / form | Engineering role |
|---|---|---|---|
| CRC-16/XMODEM | YMODEM packet | `0x1021` | UART packet error detection |
| CRC-16/IBM | OTA firmware block | `0xA001` | MQTT block payload error detection |
| CRC8 | Parameter record | implementation-defined in project | Flash parameter record consistency |

## Cryptographic Boundary

- Whole-image hash；
- Digital signature / public-key verification；
- Encrypted firmware；
- Secure Boot / Root of Trust；
- Authenticated image manifest；
- Anti-rollback version counter。

CRC 可以发现部分传输或存储错误，但不能证明映像来源，也不能抵抗恶意篡改。因此仓库不使用 `Secure OTA`、`Authenticated Firmware` 或 `Verified Boot` 描述当前架构。

[上一篇：MQTT Block Transport](mqtt-block-transport.md) · [返回 README](../README.md) · [下一篇：Failure Boundaries](failure-boundaries.md)
