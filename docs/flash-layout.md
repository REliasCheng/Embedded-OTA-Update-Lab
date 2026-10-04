# Flash Layout

本文以 GD32F407VE 的 512 KiB 内部 Flash 为参考，展示基础 IAP 与 staged update 两套布局。地址是设计模型，不是当前可构建工程的链接证明。

## 基础与 IAP

```text
0x08000000  Bootloader
0x08004000  Application
```

Bootloader 预留 16 KiB；配对 Application 使用 `0x4000` 的 vector-table offset。

## 最终 OTA

![OTA flash memory map](../assets/images/diagram/flash-memory-map.svg)

参考布局为：

| Region | Address range | Size | Role |
|---|---|---:|---|
| Bootloader | `0x08000000`–`0x08007FFF` | 32 KiB | 更新与启动控制 |
| Active App | `0x08008000`–`0x080433FF` | 237 KiB | 固定启动的 Application image |
| Staging Backup | `0x08043400`–`0x0807E7FF` | 237 KiB | 下载暂存与复制来源 |
| Update Info | `0x0807E800`–`0x0807EFFF` | 2 KiB | 更新标志 |
| Parameters | `0x0807F000`–`0x0807FFFF` | 4 KiB | 版本与参数记录 |

`Staging Backup` 与 Active App 大小相同，但 Bootloader 不从 staging 地址启动，也没有 active/pending/confirmed Slot 状态。因此它是 image storage 与 copy source，不是 A/B firmware partition。

## Link configuration 边界

未来 Application 工程必须使 Flash 起点、region length 与 `VTOR` offset 同时匹配，并把 link 上限限制在 Active App 分区内。当前仓库没有 linker 配置证明这一约束已实现。

[上一篇：Bootloader Architecture](bootloader-architecture.md) · [返回 README](../README.md) · [下一篇：Application Jump](application-jump.md)
