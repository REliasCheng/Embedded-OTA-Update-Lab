# Flash Programming Foundation

## 项目作用

提供 GD32F407VE 内部 Flash 擦除、写入和读取基础，为 Bootloader 与 IAP 的固件落盘建立底层接口。

## 技术内容

- 按 GD32F407 Flash sector 组织擦除操作；
- 按地址执行内部 Flash read/write；
- 保留 GD32 SPL、CMSIS、startup 与 Keil 工程定义。

## 关键接口

- `Common/flash_drv.*`
- `Common/fmc_operation.*`
- `course/Project/GD32F407.uvprojx`

## 工程入口

- [原始工程快照](course/)
- [Keil project](course/Project/GD32F407.uvprojx)
- [Flash layout](../../docs/flash-layout.md)

## 当前边界

本目录提供 Flash 操作基础，不包含 Bootloader/Application pair，也没有本轮自动构建或板端擦写记录。

[返回工程索引](../README.md) · [下一项：Bootloader / Application Split](../02-boot-app-split/)
