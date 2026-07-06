// GameManager.cpp
#include "GameManager.h"
#include "ConsoleUI.h"

#include <iostream>
#include <vector>

GameManager::GameManager()
    : running(true)
{
}

void GameManager::init()
{
    // TODO:
    // 1. 初始化控制台界面
    // 2. 尝试读取存档
    // 3. 如果无存档，则引导玩家创建角色
    // 4. 启动自动存档线程

    ConsoleUI::setupConsole();
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("校园 RPG 冒险游戏");

    if (!saveManager.loadGame(player, inventory, taskSystem))
    {
        std::string name;
        std::cout << "请输入你的角色名称: ";
        std::getline(std::cin, name);
        player.create(name);
    }

    autoSaveService.start(this);
}

void GameManager::run()
{
    while (running)
    {
        ConsoleUI::clearScreen();
        showMainMenu();

        int choice = ConsoleUI::readInt("请选择功能: ");
        handleMainMenu(choice);
    }
}

void GameManager::showMainMenu() const
{
    ConsoleUI::printTitle("校园 RPG 冒险游戏");

    ConsoleUI::printBox({
        "1. 查看角色信息",
        "2. 背包管理",
        "3. 校园商店",
        "4. 任务系统",
        "5. 战斗挑战",
        "6. AI 游戏助手",
        "7. 手动存档",
        "0. 退出游戏"
    });
}

void GameManager::handleMainMenu(int choice)
{
    switch (choice)
    {
    case 1:
        characterMenu();
        break;
    case 2:
        inventoryMenu();
        break;
    case 3:
        shopMenu();
        break;
    case 4:
        taskMenu();
        break;
    case 5:
        battleMenu();
        break;
    case 6:
        aiAssistantMenu();
        break;
    case 7:
        saveGame();
        ConsoleUI::pause();
        break;
    case 0:
        shutdown();
        break;
    default:
        std::cout << "无效选项，请重新输入。\n";
        ConsoleUI::pause();
        break;
    }
}

bool GameManager::saveGame()
{
    // TODO: 调用 saveManager.saveGame 保存当前游戏状态。
    return saveManager.saveGame(player, inventory, taskSystem);
}

void GameManager::shutdown()
{
    // TODO:
    // 1. 停止自动存档线程
    // 2. 最后保存一次游戏
    // 3. 设置 running = false

    autoSaveService.stop();
    saveGame();
    running = false;
}

void GameManager::characterMenu()
{
    ConsoleUI::clearScreen();
    player.showInfo();
    ConsoleUI::pause();
}

void GameManager::inventoryMenu()
{
    // TODO:
    // 1. 显示背包
    // 2. 提供使用物品、删除物品、返回等选项
}

void GameManager::shopMenu()
{
    // TODO:
    // 1. 显示商品
    // 2. 提供购买、出售、返回等选项
}

void GameManager::taskMenu()
{
    // TODO:
    // 1. 显示任务列表
    // 2. 提供接受、完成、领奖、返回等选项
}

void GameManager::battleMenu()
{
    battleSystem.startBattle(player);
}

void GameManager::aiAssistantMenu()
{
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("AI 游戏助手");

    aiAssistant.showTips(player);
    std::cout << "\n推荐行动: "
              << aiAssistant.suggestAction(player, inventory)
              << "\n";

    ConsoleUI::pause();
}