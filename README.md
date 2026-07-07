# CampusRPG

一个基于 C++17 的命令行校园 RPG 项目，包含角色、背包、商店、任务、战斗、存档和 AI 助手功能。

## 构建环境

- Windows
- MSYS2 UCRT64
- `g++` with C++17 support
- `libcurl` for AI network requests
- CMake 3.16+

当前项目默认按你本机的 MSYS2 UCRT64 工具链使用：
- `E:\c++\msys2\ucrt64\bin\g++.exe`

## 方式一：用 CMake 构建

配置：

```powershell
cmake -S . -B build
```

编译：

```powershell
cmake --build build --config Release
```

说明：
- `CMakeLists.txt` 会自动编译 `main.cpp` 和 `src/*.cpp`
- 如果检测到 `libcurl`，会自动启用 `ENABLE_CURL_AI`
- 如果没有检测到 `libcurl`，项目仍可编译，但 AI 联网功能会关闭

## 方式二：直接用 g++ 构建

```powershell
& 'E:\c++\msys2\ucrt64\bin\g++.exe' `
  -g `
  -DENABLE_CURL_AI `
  -Iinclude `
  -Isrc `
  main.cpp `
  src\AIAssistant.cpp `
  src\AutoSaveService.cpp `
  src\BattleSystem.cpp `
  src\Character.cpp `
  src\ConsoleUI.cpp `
  src\Enemy.cpp `
  src\EnvLoader.cpp `
  src\GameManager.cpp `
  src\Inventory.cpp `
  src\Item.cpp `
  src\SaveManager.cpp `
  src\Shop.cpp `
  src\Task.cpp `
  src\TaskSystem.cpp `
  -lcurl `
  -o main.exe
```

如果 `main.exe` 正在运行，链接器无法覆盖输出文件。先关闭正在运行的程序，再重新编译。

## VS Code 构建

项目已经包含 `.vscode/tasks.json`，默认构建任务会：
- 编译所有源文件
- 定义 `ENABLE_CURL_AI`
- 链接 `-lcurl`

直接在 VS Code 中运行默认 Build Task 即可。

## AI 功能配置

AI 助手通过 `DEEPSEEK_API_KEY` 环境变量调用 DeepSeek 接口。

项目启动时会优先读取根目录 `.env` 文件。可以参考 `.env.example`：

```env
DEEPSEEK_API_KEY=your_api_key_here
```

建议做法：

1. 复制 `.env.example` 为 `.env`
2. 填入真实的 API Key
3. 重新编译并运行程序

说明：
- 如果 `.env` 不存在，程序会继续读取系统环境变量
- 如果系统环境变量已存在，同名值不会被 `.env` 覆盖
- 如果没有可用的 API Key，AI 助手会退回本地规则建议

## 运行

```powershell
.\main.exe
```

主菜单中的 `6. AI 游戏助手` 会显示本地提示，并在可用时调用 DeepSeek 获取建议。
