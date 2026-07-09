# CampusRPG

一个基于 C++17 的命令行校园 RPG 项目，包含角色成长、背包、商店、任务、存档、自动存档、AI 助手，以及重构后的卡牌战斗系统。

## 功能概览

- 角色系统：等级、生命、攻击、防御、金币、经验
- 背包系统：食物、药品、装备
- 商店系统：购买、出售物品
- 任务系统：任务加载、进度广播、奖励领取
- 自由战斗：单场遭遇战
- 冒险挑战：多阶段连续战斗
- 卡牌战斗：攻击 / 防御 / 回复三类手牌，带先后手博弈与一次性指令
- AI 系统：
  - 游戏主菜单里的 AI 助手
  - 战斗中的敌方 AI，支持远程模型与本地降级

## 环境要求

- Windows
- Python 3
- 以下二选一
  - Visual Studio 2022 / MSVC
  - MSYS2 MinGW-w64 `g++`

## 使用 CMake 构建

配置工程：

```powershell
cmake -S . -B build
```

编译：

```powershell
cmake --build build --config Release
```

如果旧的 `build` 目录缓存了错误配置，可以删除后重建：

```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build
cmake --build build --config Release
```

## 使用 g++ 直接编译

适用于 MSYS2 MinGW-w64 环境，例如 `mingw64.exe` 或 `ucrt64.exe` 终端：

```bash
g++ -std=c++17 -O2 -Iinclude -Isrc \
  main.cpp \
  src/AdventureManager.cpp \
  src/AIAssistant.cpp \
  src/AutoSaveService.cpp \
  src/BattleAI.cpp \
  src/BattleDeck.cpp \
  src/BattleMapper.cpp \
  src/BattleSystem.cpp \
  src/BattleTypes.cpp \
  src/Character.cpp \
  src/ConsoleUI.cpp \
  src/Enemy.cpp \
  src/EnemyFactory.cpp \
  src/EnvLoader.cpp \
  src/GameManager.cpp \
  src/Inventory.cpp \
  src/Item.cpp \
  src/SaveManager.cpp \
  src/Shop.cpp \
  src/SimpleJson.cpp \
  src/Task.cpp \
  src/TaskSystem.cpp \
  -o CampusRPG.exe
```

如果你在 PowerShell 里直接调用 MSYS2 的 `g++`，可以写成：

```powershell
& "C:\msys64\mingw64\bin\g++.exe" -std=c++17 -O2 -Iinclude -Isrc `
  main.cpp `
  src\AdventureManager.cpp `
  src\AIAssistant.cpp `
  src\AutoSaveService.cpp `
  src\BattleAI.cpp `
  src\BattleDeck.cpp `
  src\BattleMapper.cpp `
  src\BattleSystem.cpp `
  src\BattleTypes.cpp `
  src\Character.cpp `
  src\ConsoleUI.cpp `
  src\Enemy.cpp `
  src\EnemyFactory.cpp `
  src\EnvLoader.cpp `
  src\GameManager.cpp `
  src\Inventory.cpp `
  src\Item.cpp `
  src\SaveManager.cpp `
  src\Shop.cpp `
  src\SimpleJson.cpp `
  src\Task.cpp `
  src\TaskSystem.cpp `
  -o CampusRPG.exe
```

说明：

- 上面的 `C:\msys64\mingw64\bin\g++.exe` 只是示例路径，请改成你本机的实际路径
- 当前 AI 功能依赖 Python 脚本，不需要额外链接 `libcurl`

## 运行

如果使用 Visual Studio 这类多配置生成器：

```powershell
.\build\Release\CampusRPG.exe
```

如果使用单配置生成器：

```powershell
.\build\CampusRPG.exe
```

如果使用 `g++` 直接编译：

```powershell
.\CampusRPG.exe
```

## 卡牌战斗系统

当前战斗系统已从原本的单一“攻击”交互改为卡牌回合制。

### 核心规则

- 每回合双方各自抽 3 张手牌
- 你必须从当前手牌中打出 1 张
- 卡牌类型：
  - 攻击牌
  - 防御牌
  - 回复牌
- 双方各有 1 次“指令”机会，可以无视手牌强制选择一种动作
- 支持先后手机制与公开意图博弈

### 伤害规则

当前项目不是直接照搬参考项目的纯 `ATK` 伤害，而是兼容现有属性体系：

```text
基础伤害 = max(1, 攻击方ATK - 防守方DEF)
```

在此基础上再叠加：

- 元素克制：火克草、草克水、水克火
- 防御减伤
- 易伤层数：每层额外承受 +50% 伤害

### 防御规则

- 防御成功反制攻击时：
  - 对手下回合缴械
  - 对手永久失去一张攻击牌
- 防御落空时：
  - 自己获得 1 层易伤
  - 自己永久失去一张防御牌

### 冒险模式

- 冒险模式现在也走同一套卡牌战斗系统
- 每个阶段仍然采用“连续单挑”结构，不是多目标同场混战
- `AdventureManager` 会把当前阶段实际生成的敌人列表直接传给 `BattleSystem`
  - 这样可以保证“展示的敌人”和“实际交战的敌人”一致

## 战斗 AI

战斗敌方 AI 支持两层结构：

- 远程模型 AI
- 本地 C++ / Python fallback 策略

### 远程 AI 配置

通过环境变量配置：

```env
BATTLE_AI_API_URL=your_chat_completions_url
BATTLE_AI_API_KEY=your_api_key
BATTLE_AI_MODEL=your_model_name
```

说明：

- `BATTLE_AI_API_URL` 需要是兼容聊天补全接口的地址
- `BATTLE_AI_MODEL` 由你自己使用的服务端模型名决定
- 如果这些变量缺失，战斗 AI 会自动退回本地策略

### AI 脚本位置

战斗 AI 脚本位于：

```text
scripts/battle_ai.py
```

程序会通过项目路径解析查找它，因此通常不需要手动复制到 `build` 目录。

### 控制台输出

战斗中会把 AI 调用信息输出到控制台，包括：

- 远程 AI 是否调用成功
- 流式调用是否失败并回退为非流式
- 最终是否降级到本地策略
- AI 的分析 / 思考文本

如果远程模型支持流式返回，当前版本会尽量以流式方式把分析过程打印到控制台。

### 降级策略

出现以下任一情况时，战斗不会报错退出，而是自动切换到本地策略：

- 没有安装 Python
- 没有配置 `BATTLE_AI_API_URL / KEY / MODEL`
- `scripts/battle_ai.py` 不存在
- 远程接口超时、返回格式异常或调用失败

## AI 助手

主菜单中的 AI 助手与战斗 AI 是两套独立能力。

AI 助手仍然使用：

```text
scripts/ai_helper.py
```

并依赖它自己的环境变量配置。它负责根据玩家当前状态给出主菜单层面的行动建议，不参与战斗对局决策。

## 代码结构

与战斗相关的主要文件：

- `include/BattleTypes.h`
- `include/BattleDeck.h`
- `include/BattleAI.h`
- `include/BattleMapper.h`
- `include/BattleSystem.h`
- `src/BattleTypes.cpp`
- `src/BattleDeck.cpp`
- `src/BattleAI.cpp`
- `src/BattleMapper.cpp`
- `src/BattleSystem.cpp`
- `scripts/battle_ai.py`

职责划分：

- `BattleTypes`：战斗枚举、卡牌结构、战斗快照
- `BattleDeck`：卡组、抽牌、永久削牌
- `BattleAI`：远程 AI 调用与本地决策
- `BattleMapper`：把 `Character / Enemy` 映射为战斗态
- `BattleSystem`：战斗主循环与回合结算

## 当前限制

- 战斗内背包 / 物品使用目前只预留了接口，尚未真正接入 `Inventory`
- 元素类型目前仍主要靠名称推断，不是正式数据字段
- 远程 AI 的流式输出是否完全可用，取决于你的接口是否兼容当前脚本的流式格式解析

## 建议的后续改进

- 给角色和敌人增加显式 `element` 字段，替代名称推断
- 把战斗内物品正式接入指令分支
- 继续微调 Boss / Elite / Minion 的卡牌战斗映射数值
- 如果远程接口格式稳定，可以进一步增强流式思考输出解析
