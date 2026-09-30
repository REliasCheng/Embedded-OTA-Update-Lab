# MQTT Block Transport

最终 OTA 系统由 GD32F407VE 通过 USART2 控制 ESP8266 类 AT 模组。模组负责 Wi-Fi 与 MQTT 连接，MCU 侧 OTA 状态机通过 Aliyun IoT topic 处理元数据并请求 firmware block；该链路只承担更新控制与固件传输。

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

公开快照中的 Wi-Fi、endpoint、device identifier、MQTT credential 和 topic literal 已替换为占位符。替换不改变命令状态机、block request、CRC 或 Flash 数据路径。

## Block transport

请求参数由 `streamId`、`fileId`、固件大小和 offset 组成。下载进度保存在 RAM 中，block CRC mismatch 和通信失败分别有当前会话内的重试计数。

按 offset 请求 block 不等于 persistent resume：复位后没有从 Flash 元数据恢复 block progress 的实现证据。

## Transport security

主线连接配置使用 MQTT 1883，未发现 TLS transport。MQTT 鉴权字段也不等同于 firmware signature 或 image encryption。

## 工程入口

- [MQTT OTA System](../projects/05-mqtt-ota-system/)
- [Image Integrity](image-integrity.md)
- [Failure Boundaries](failure-boundaries.md)

[上一篇：OTA Control Plane](ota-control-plane.md) · [返回 README](../README.md) · [下一篇：Image Integrity](image-integrity.md)
