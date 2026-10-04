# OTA Control Plane

OTA Control Plane 决定“是否进入更新”，Firmware Block Transport 负责“如何取得映像数据”。两者在架构中属于不同职责。

## Application 侧流程

```text
OTA metadata notification
          ↓
Parse version / size / stream and file identifiers
          ↓
Compare current and received version strings
          ↓
User update decision
          ↓
Write update flag (0xABCD)
          ↓
System reset → Bootloader
```

简单的字符串相等比较只能决定版本是否不同，不能提供 SemVer 数值排序、最低允许版本、硬件兼容矩阵或 Anti-rollback counter。

## Bootloader 侧交接

参考 Bootloader 读取 Update Info 区中的更新标志：无更新时进入固定 Application，有更新时进入下载状态机。更新标志与参数记录用于流程控制，不是 signed manifest 或原子事务日志。

## 与传输层的边界

Wi-Fi、AT command 与 MQTT 只为 OTA 提供 control/data transport。Metadata notification 和 update intent 属于 control plane；firmware block request/response 属于 data transport。网络连接本身不是该仓库要展开的通用 Wireless Architecture。

[上一篇：YMODEM IAP](ymodem-iap.md) · [返回 README](../README.md) · [下一篇：MQTT Block Transport](mqtt-block-transport.md)
