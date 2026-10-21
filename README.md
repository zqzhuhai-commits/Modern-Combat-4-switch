# Modern Combat 4: Zero Hour — Nintendo Switch port

《现代战争4》(Gameloft, Android) 的 Switch 大气层移植。AArch32 原生执行（Gameloft GL2JNI 引擎, shadowguardian-switch 同路线），无需模拟器/JIT。

- **Title ID**: 0x0100F53447550002
- **输出分辨率**: 1280x720 (1:1 原生)
- **运行环境**: Switch 大气层 (CFW) + Tinfoil / DBI 安装 NSP

## 安装

### 1. 安装 NSP
用 Tinfoil / DBI / 大气层安装器安装 `Modern-Combat-4.nsp`。

### 2. 放置游戏数据（必须）
游戏本体数据放在 SD 卡 `sdmc:/switch/mc4_nx/`：

```
sdmc:/switch/mc4_nx/
├── libModernCombat4.so          # 游戏主库 (24.8MB, 来自 Android 安装包)
├── obb/                         # 数据包目录（推荐）
│   ├── main.1100.com.gameloft.android.ANMP.GloftM4HM.obb   # 主数据包 2.05GB
│   └── patch.12222.com.gameloft.android.ANMP.GloftM4HM.obb # 补丁包 15MB
├── assets/                      # 原始资源目录（可选，OBB 缺失时回退）
└── mc4.cfg                      # 游戏配置（language=cn 等）
```

> 数据来源：原版 Android APK + OBB（`main.1100....obb` + `patch.12222....obb`）。请从你拥有的正版安装包提取，本仓库不含版权数据。

## 键位映射（Joy-Con / Pro 手柄）

| 手柄 | 动作 |
|---|---|
| 左摇杆 | 移动 |
| 右摇杆 | 瞄准（鼠标） |
| A | 射击 |
| ZL | 瞄准 |
| ZR | 射击 |
| X | 动作键 |
| Y | 抓取 |
| B / Plus | 返回 |
| Minus | 菜单 |

触摸屏输入透传支持。

## 构建

依赖：devkitPro 容器 (devkitARM 15.x)、libnx32 (AArch32 libnx)、Mesa 20 portlibs32、miniz 3.0.2。

```sh
./build.sh            # chroot 内构建 ELF32 → NSO → ExeFS NSP
node mc4-package.mjs build/game/mc4_game.nso build/game/mc4_game.npdm Modern-Combat-4.nsp  # 标准 NSP (hacbrewpack)
```

## 源码结构

- `source/` — 移植层（加载器 main.c、JNI 绑定 mc4_api.c/mc4_jni.c、AAssetManager 垫片 mc4_asset.c、GL/ATC 兼容 mc4_gl.c/mc4_atc.c、手柄 mc4_input.c）
- `runtime/` — AArch32 运行层框架（自 shadowguardian-switch, MIT）
- `tools/` — imports.cfg / gen_imports.py / elf32_to_nso.py
- `portlibs32/` — Mesa20 GLES2 静态库 (AArch32)

MIT License. 游戏内容归 Gameloft 所有。
