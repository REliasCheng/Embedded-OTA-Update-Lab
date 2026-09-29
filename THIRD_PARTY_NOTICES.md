# Third-Party Notices

`projects/**/course/` 保存筛选后的固件升级参考工程。除 Manifest 明确标记的两份 `SANITIZED_DERIVATIVE` 外，迁移文件保持原始字节不变；原有版权头和许可证继续适用。

仓库根目录的 MIT License 仅适用于本仓库新增的 Markdown 文档、自有 SVG 与后续明确标注的原创代码，不覆盖迁移的源资料或第三方组件。

实际迁移的第三方内容包括：

- GigaDevice GD32F4xx device support、Standard Peripheral Library、启动文件及其保留的版权与许可声明；
- ARM CMSIS core support 及其保留的版权与许可声明；
- Keil MDK-ARM 工程定义，按相应工具与器件包条款使用。

Bootloader、Flash、YMODEM、ESP8266 AT 与 OTA 应用层文件作为所提供源资料的筛选快照保留；本仓库不通过根目录 License 对其重新授权。构建日志、IDE 用户状态、固件二进制、Office/PDF/XMind、draw.io 和许可不明视觉资源均未纳入公开快照。

迁移来源、目标路径、模式与 SHA256 记录在 [SOURCE_SELECTION_MANIFEST.csv](SOURCE_SELECTION_MANIFEST.csv)，迁移后复核结果见 [MIGRATION_HASH_VERIFICATION.csv](MIGRATION_HASH_VERIFICATION.csv)。
