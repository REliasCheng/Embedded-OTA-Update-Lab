# 当前实现边界 | Failure Boundaries

本页记录当前实现的 update-state 与恢复边界，区分已存在的分区/重试机制和尚无证据支持的可靠性能力。

## Staging 与启动

固件块先写入 Staging Backup，再复制到固定 Active App。Bootloader 只从 `0x08008000` 启动 Application；没有两个独立可启动 Slot，也没有 active/pending/confirmed image state。

因此当前结构不能描述为：

- A/B firmware update；
- Dual-bank boot；
- Automatic rollback。

## 恢复状态

block offset、当前 block 和重试计数主要保存在 RAM。资料中未形成跨复位的 download progress、copy progress journal 或 confirmed-boot state，所以没有 persistent resume 或 guaranteed power-loss recovery 证据。

## 静态代码观察

以下条目来自静态代码核对，未通过本轮构建或板端故障注入复现：

- Active App 擦除范围与 237 KiB 逻辑分区不完全一致；
- copy loop 使用 `image_size / 1024 + 1`，整 1 KiB 倍数边界需要复核；
- 下载大小缺少对完整 Staging Backup 上限的统一拒绝路径；
- update copy 返回后，版本/标志更新的失败路径需要单独验证；
- Application link region 没有被限制到 Active App 分区大小。

这些观察保留为后续构建、板端与故障注入验证入口，不在文档阶段修改迁移源码。

## 当前没有证据支持的结论

- Hardware update success；
- Corrupted-image rejection test；
- Power-loss recovery test；
- Rollback after failed boot；
- Production-ready OTA。

[上一篇：Image Integrity](image-integrity.md) · [返回 README](../README.md) · [下一篇：Development Environment](development-environment.md)
