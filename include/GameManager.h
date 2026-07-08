// GameManager.h
#ifndef GAME_MANAGER_H_GAME_MANAGER_H
#define GAME_MANAGER_H_GAME_MANAGER_H

#include "Character.h"
#include "Inventory.h"
#include "Shop.h"
#include "TaskSystem.h"
#include "BattleSystem.h"
#include "AdventureManager.h"
#include "SaveManager.h"
#include "AutoSaveService.h"
#include "AIAssistant.h"
#include "CheatManager.h"

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
    AdventureManager adventureManager;
    SaveManager saveManager;
    AutoSaveService autoSaveService;
    AIAssistant aiAssistant;
    CheatManager cheatManager;

    bool running;
    bool shopUnlocked;

public:
    GameManager();

    void init();
    void run();
    void showMainMenu() const;
    void handleMainMenu(int choice);
    bool saveGame();
    void shutdown();

private:
    void broadcastTaskEvent(const TaskMessage& msg);
    void characterMenu();
    void inventoryMenu();
    void shopMenu();
    void taskMenu();
    void battleMenu();
    void adventureMenu();
    void aiAssistantMenu();
    void cheatMenu();

    // ---- 任务系统二级菜单 ----
    void taskSubMenu(QuestType type);
};

#endif
