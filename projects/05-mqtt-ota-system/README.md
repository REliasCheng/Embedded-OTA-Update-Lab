# MQTT OTA System

## 项目作用

将 Application 侧版本查询与 Bootloader 侧固件下载、Staging Flash 和固定 Application 区复制组织为配对 OTA 系统。

## 系统配对

```text
application/course/  metadata, version state, update intent
bootloader/course/   MQTT block download, CRC, staging, copy, boot
```

网络链路由 GD32F407VE 通过 USART2 控制 ESP8266 类 AT 模组；固件块经 Aliyun IoT MQTT OTA topic 传输。公开派生文件只将网络和云端 credential literal 替换为占位符，控制流与数据处理保持不变。

## 关键机制

- `0x08008000` 固定 Active App 起点；
- 256-byte block request/response；
- OTA block CRC-16/IBM；
- Active App + Staging Backup；
- update flag、version parameter 与固定地址启动。

## 工程入口

- [OTA Application](application/course/)
- [OTA Bootloader](bootloader/course/)
- [MQTT block transport](../../docs/mqtt-block-transport.md)
- [Flash layout](../../docs/flash-layout.md)
- [Image integrity](../../docs/image-integrity.md)

[上一项：OTA Control](../04-ota-control/) · [返回工程索引](../README.md)
