# 架构边界 | Failure Boundaries

本页记录架构模型的 update-state 与恢复边界，区分设计元素和尚无实现证据支持的可靠性能力。

## Staging 与启动

模型先把固件块写入 Staging Backup，再复制到固定 Active App。Bootloader 只从 `0x08008000` 启动 Application；它没有两个独立可启动 Slot，也没有 active/pending/confirmed image state。

因此当前结构不能描述为：

- A/B firmware update；
- Dual-bank boot；
- Automatic rollback。

## 恢复状态

若 block offset、当前 block 和重试计数只保存在 RAM，则无法支持跨复位的 persistent resume。当前仓库没有 download journal、copy journal 或 confirmed-boot 实现证据。

## Future Verification

- 擦除与写入范围必须严格落在目标分区内。
- block 和 copy loop 必须覆盖整倍数与尾包边界。
- 下载大小必须在擦写前检查完整 Staging 上限。
- 版本、更新标志和 copy 失败路径需要故障注入验证。
- Link region 必须限制在 Active App 分区大小内。

## 当前没有证据支持的结论

- Hardware update success；
- Corrupted-image rejection test；
- Power-loss recovery test；
- Rollback after failed boot；
- Production-ready OTA。

[上一篇：Image Integrity](image-integrity.md) · [返回 README](../README.md) · [下一篇：Development Environment](development-environment.md)
