// GameManager.h
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Character.h"
#include "Inventory.h"
#include "Shop.h"
#include "TaskSystem.h"
#include "BattleSystem.h"
#include "SaveManager.h"
#include "AutoSaveService.h"
#include "AIAssistant.h"

// ============================================================
// GameManager
// ------------------------------------------------------------
// 游戏总控制器。
// 负责初始化游戏、主循环、菜单分发、存档、退出等。
// ============================================================

class GameManager
{
private:
    Character player;
    Inventory inventory;
    Shop shop;
    TaskSystem taskSystem;
    BattleSystem battleSystem;
    SaveManager saveManager;
    AutoSaveService autoSaveService;
    AIAssistant aiAssistant;

    bool running;

public:
    GameManager();

    // 初始化游戏。
    // 包括控制台设置、读取存档、创建角色、启动自动存档等。
    void init();

    // 主循环。
    void run();

    // 显示主菜单。
    void showMainMenu() const;

    // 根据用户输入处理主菜单选项。
    void handleMainMenu(int choice);

    // 保存游戏。
    bool saveGame();

    // 关闭游戏。
    void shutdown();

private:
    // 角色管理菜单。
    void characterMenu();

    // 背包管理菜单。
    void inventoryMenu();

    // 商店菜单。
    void shopMenu();

    // 任务菜单。
    void taskMenu();

    // 战斗菜单。
    void battleMenu();

    // AI 助手菜单。
    void aiAssistantMenu();
};

#endif