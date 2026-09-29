# Bootloader / Application Split

## 项目作用

将启动逻辑与 Application 分离：Bootloader 位于 `0x08000000`，配对 Application 位于 `0x08004000`。

## 系统配对

```text
bootloader/course/   fixed boot entry and application jump
application/course/  application linked at 0x08004000, VTOR offset 0x4000
```

Bootloader 从 Application 起点读取初始 MSP 和 Reset Handler；Application 在早期初始化中设置向量表偏移。两个工程必须配套查看。

## 关键接口

- Bootloader `User/main.c`：`BootToApp()`
- Application `Common/iap_driver.c`：`nvic_vector_table_set(...)`
- 两个 `Project/GD32F407.uvprojx`

## 工程入口

- [Bootloader source snapshot](bootloader/course/)
- [Application source snapshot](application/course/)
- [Application jump](../../docs/application-jump.md)

[上一项：Flash Basics](../01-flash-basics/) · [返回工程索引](../README.md) · [下一项：YMODEM IAP](../03-ymodem-iap/)
