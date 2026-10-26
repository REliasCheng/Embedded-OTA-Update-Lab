# 架构边界 | Failure Boundaries

本页区分已运行的 Host OTA 软件模型与仍未实现的 GD32 目标 Bootloader。模拟 Flash 的故障注入结果不能直接推断真实设备的掉电行为。

## Staging 与启动

当前 [Host 更新流程](host-update-flow.md)将经过分块校验的数据写入模拟 Staging，读回并校验整包 CRC32，再复制到模拟 Active 并再次读回校验。写入、擦除、复制、读取和 Journal 写入均有可重复的故障注入测试；损坏映像会被拒绝。

`ota_update_boot_eligible()` 只在 Host 状态为 `APPLIED` 且 Active 内容仍与 manifest CRC32 一致时返回可启动资格。它没有验证 Cortex-M 向量、MSP、Reset Handler 或实际启动。参考布局中的 Bootloader 从固定 Application 地址交接执行权；Staging 不是第二套可启动 Slot。因此不能将当前实现描述为 A/B 更新、双 Bank 启动或自动回滚。

## Journal 与恢复

Host 核心已有两个交替擦除单元的 Journal，记录 `RECEIVING`、`RECEIVED`、`READY`、`APPLYING`、`APPLIED`，并校验记录的 CRC32 与映像元数据。`ota_update_resume()` 在模拟复位后读取有效记录及模拟 Flash 内容，恢复保守状态：

- `READY` 或 `APPLYING` 只有在 Staging 整包 CRC32 仍正确时，才能进入需要显式调用 `ota_update_retry_apply()` 的恢复状态；不会自动继续复制。
- `APPLIED` 还须通过 Active 整包 CRC32 读回校验，Host 模型才接受该状态。
- 没有有效记录或记录损坏时，不会被当作成功安装。

Journal **不保存已接收的块数或下载进度**。`RECEIVING`、`RECEIVED` 阶段发生中断后，不能从原偏移续传；必须通过 `ota_update_begin()` 重新开始接收。部分写入或复制失败也不会被报告为原子成功。

## 仍需目标与设备验证

- 参考 Flash 分区是否符合 GD32F407VE 的实际擦除扇区、链接边界和启动向量要求。
- 真实 Flash 在断电、部分擦除或部分写入时的行为，以及 Journal 两槽记录在该设备上的持久化保证。
- 目标端 Bootloader、传输协议、应用跳转、失败后的外部恢复路径和实机运行结果。

单 Active 加 Staging 不提供原子升级或自动回滚。Host CRC32 用于检测意外损坏，不提供固件来源认证。当前没有真实设备掉电安全、板端成功启动或生产级 OTA 的证据。

[上一篇：Image Integrity](image-integrity.md) · [返回 README](../README.md) · [下一篇：Development Environment](development-environment.md)
