# OTA Control Plane

## 项目作用

整理 Application 侧的 OTA 元数据处理、版本状态、更新确认、更新标志和复位交接，不承担 Bootloader 内的固件复制流程。

## 软件流程

```text
OTA Metadata
    ↓
Version String Comparison
    ↓
Update Intent / Flag
    ↓
System Reset → Bootloader
```

版本判断只检查字符串是否相等；当前工程没有语义版本排序或 Anti-rollback 规则。该目录聚焦 Application 侧 control plane，不代表固件块已经在此工程中完成下载与写入。

## 工程入口

- [Application source snapshot](application/course/)
- [OTA Control Plane](../../docs/ota-control-plane.md)
- [Failure Boundaries](../../docs/failure-boundaries.md)

[上一项：YMODEM IAP](../03-ymodem-iap/) · [返回工程索引](../README.md) · [下一项：MQTT OTA](../05-mqtt-ota-system/)
