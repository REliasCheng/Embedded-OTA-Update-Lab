# MQTT Block Transport

参考模型由 GD32F407VE 通过 UART 控制 ESP8266 类 AT 模组。模组负责 Wi-Fi 与 plain MQTT 1883 连接，MCU 侧状态机处理元数据并请求 firmware block；当前默认分支不包含实现或云端运行证据。

## 数据路径

```text
Aliyun IoT OTA
      ↓ MQTT topic (TCP 1883)
ESP8266-class AT module
      ↓ USART2 / AT response
GD32 OTA state machine
      ↓ 256-byte block + CRC-16/IBM
Staging Backup in internal Flash
```

当前默认分支不包含 Wi-Fi、endpoint、device identifier、MQTT credential 或 topic literal。未来实现必须通过安全配置提供这些值。

## Block transport

请求模型可由 `streamId`、`fileId`、固件大小和 offset 组成。若下载进度只保存在 RAM 中，重试只覆盖当前会话。

按 offset 请求 block 不等于 persistent resume：复位后没有从 Flash 元数据恢复 block progress 的实现证据。

## Transport security

本文只描述 MQTT 1883，没有 TLS transport 实现。MQTT 鉴权字段也不等同于 firmware signature 或 image encryption。

## Related Documentation

- [Image Integrity](image-integrity.md)
- [Failure Boundaries](failure-boundaries.md)

[上一篇：OTA Control Plane](ota-control-plane.md) · [返回 README](../README.md) · [下一篇：Image Integrity](image-integrity.md)
