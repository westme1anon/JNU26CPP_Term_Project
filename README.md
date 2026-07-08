# CampusRPG

一个基于 C++17 的命令行校园 RPG 项目，包含角色成长、背包、商店、任务、战斗、存档和 AI 助手功能。

## 环境要求

- Windows
- Visual Studio 2022 / MSVC
- CMake 3.16+
- Python 3

## 构建

配置工程：

```powershell
cmake -S . -B build
```

编译：

```powershell
cmake --build build --config Release
```

如果旧的 `build` 目录缓存了错误配置，可以删除后重新生成：

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build
cmake --build build --config Release
```

## AI 助手

联网 AI 不再依赖 `libcurl`，而是改为：

- C++ 将当前游戏状态写入临时文件
- 游戏调用 `scripts/ai_helper.py`
- Python 脚本负责向 DeepSeek 发送 HTTP 请求
- 如果 Python、脚本、API Key 或网络请求不可用，程序会自动回退到本地规则建议

在 `.env` 或系统环境变量中设置 API Key：

```env
DEEPSEEK_API_KEY=your_api_key_here
```

Python 脚本只使用标准库，不需要额外执行 `pip install`。

### 本地回退策略

当出现以下情况时，游戏不会报错退出，而是自动切换到本地建议模式：

- 没有安装 Python
- 没有配置 `DEEPSEEK_API_KEY`
- `scripts/ai_helper.py` 不存在
- Python 脚本请求失败或超时

切换后，AI 助手会根据玩家生命值、等级、金币和背包状态给出本地建议。

## 运行

如果使用 Visual Studio 这类多配置生成器：

```powershell
.\build\Release\CampusRPG.exe
```

如果使用单配置生成器：

```powershell
.\build\CampusRPG.exe
```
