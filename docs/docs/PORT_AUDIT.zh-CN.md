# 孤侠魅影 Switch 移植核对（2026-10-21）

## 结论

仓库当前游戏运行层尚未移植完成。可以独立构建与打包原生前端；不能将它标为可玩游戏。此次已使用用户此前提供的 PortMaster ZIP 和 M3 源码核对，不需要用户重复提供这些资料。

## 已核实证据

- 当前仓库最初提交为 `7556ebd`；根目录旧 README 和验收记录实际描述 另一款游戏。
- 当前 `launcher/build.sh` 使用旧 `PAYLOAD=tasm2_nx`；本次修正为 `shadowguardian_nx`。
- 当前 `source/tasm2_boot.c` 在日志中使用旧安装目录；本次修正为 Shadow Guardian 目录。
- 用户原始库大小为 3,821,784 字节，ELF32 ARM little-endian，SHA256 为 `ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9`。
- 实际导出为 `Java_com_gameloft_android_ANMP_GloftSGHP_ML_...` 系列；存在 `GameRenderer_nativeInit/nativeRender/nativeResize` 和 `GameGLSurfaceView_nativeOnTouch/nativePause/nativeResume`。
- 当前启动代码调用的 10 种 `Java_com_gameloft_glf_GL2JNILib_...` 导出均不在该原库中。移植运行层即使编译成功，也会在这些入口绑定阶段失败。完整列表见 JSON 审计。
- 原始 ZIP 的 `Gameloft/Games/GloftSGHP/` 实际含 3,684 个文件，共 524,375,398 字节，其中 3,680 个非空文件。这是对本次 ZIP 的统计；原包历史 BUILD-INFO 的数字不同，不能用于当前完整性判定。
- M3 源码是仅加载器诊断阶段，其 README 明确尚未执行游戏构造函数、JNI 或渲染循环。存在正确命名的接口绑定草稿，尚不足以证明 JNI 实现或启动顺序已验证。

## 本次交付

新增独立 `frontend/`，编译为 AArch64 原生前端和实际安装 NSP，不链接或执行不匹配的游戏运行层。前端会核对上述原库的 SHA256；素材只做有界文件统计，不能证明数据完整性。

NSP Title ID 为 `0100F53447550000`，与当前 AArch32 运行层模板 ID `055A3A4744DC9101`、此前 M3 测试包 ID `0100534744494000` 均不同。不会通过 ExeFS override 改写其他应用。前端仅在用户按 X 时创建指定目录并写 `frontend.log`。

## 运行层后续实施顺序

1. 以已审计原始库为基准，重新实现独立 SG API 绑定；移除启动流程对 旧模板 导出和对象布局的依赖。
2. 根据原库反汇编与已知参考实现逐项确认 JNI 方法签名、返回对象及初始化顺序。M3 的空 JNI 表无法运行游戏，不能直接打开其初始化开关。
3. 整理原始素材路径映射，测试大小写、文件查找、字体语言、解包/纹理格式；保留存档目录。
4. 接入 EGL/GLES、音频、输入和生命周期。游戏资源库同时依赖 GLES1 和 GLES2，必须明确各导入的兼容实现。
5. 先取得 Switch 真机日志与首帧，再测试标题界面、进入关卡、声音、手柄、存档重启和 HOME/休眠恢复；最后制作可玩 NSP。

当前缺少的是已完成并验证的运行层，不是压缩包重命名或单纯的 NSP 打包步骤。本次没有生成伪装成可玩版的包。
