// GameManager.cpp
#include "GameManager.h"
#include "ConsoleUI.h"

#include <iostream>
#include <string>

GameManager::GameManager()
    : running(true)
{
}

void GameManager::init()
{
    // 1. 初始化控制台界面
    ConsoleUI::setupConsole();
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("校园 RPG 冒险游戏");
    // 2. 预加载商店、任务和敌人数据
    shop.loadGoods();
    taskSystem.loadTasks();
    battleSystem.loadEnemies();
    // 3. 尝试读取存档；若无存档则创建新的角色
    if (!saveManager.loadGame(player, inventory, taskSystem))
    {
        std::string name;
        std::cout << "请输入你的角色名称: ";
        std::getline(std::cin, name);
        player.create(name);
    }
    // 4. 启动自动存档线程
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
        std::cout << "游戏已保存。\n";
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
    // 调用 SaveManager 保存当前游戏状态
    return saveManager.saveGame(player, inventory, taskSystem);
}

void GameManager::shutdown()
{
    // 停止自动存档线程，保存游戏并退出
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
    while (true)
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("背包管理");
        if (inventory.isEmpty())
        {
            std::cout << "你的背包是空的，没有任何物品。\n";
            ConsoleUI::pause();
            return;
        }
        inventory.showItems();
        std::cout << "\n1. 使用物品\n";
        std::cout << "2. 丢弃物品\n";
        std::cout << "0. 返回上一级\n";
        int choice = ConsoleUI::readInt("请选择操作: ");
        switch (choice)
        {
        case 1:
        {
            // 使用物品
            int idx = ConsoleUI::readInt("选择要使用的物品编号: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                if (inventory.useItem(idx, player))
                {
                    std::cout << "物品使用成功！\n";
                }
                else
                {
                    std::cout << "无法使用该物品。\n";
                }
            }
            else
            {
                std::cout << "无效选项，请重新输入。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            // 丢弃物品（出售物品获得一半价格）
            int idx = ConsoleUI::readInt("选择要丢弃的物品编号: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                std::unique_ptr<Item> soldItem = inventory.sellItem(idx);
                if (soldItem)
                {
                    int goldGain = soldItem->getPrice() / 2;
                    player.gainGold(goldGain);
                    std::cout << "已出售 " << soldItem->getName() << "，获得 " << goldGain << " 金币。\n";
                }
                else
                {
                    std::cout << "无法出售该物品。\n";
                }
            }
            else
            {
                std::cout << "无效选项，请重新输入。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 0:
            return;
        default:
            std::cout << "无效选项，请重新输入。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

void GameManager::shopMenu()
{
    while (true)
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("校园商店");
        shop.showGoods();
        std::cout << "\n1. 购买商品\n";
        std::cout << "2. 出售物品\n";
        std::cout << "0. 返回上一级\n";
        int choice = ConsoleUI::readInt("请选择操作: ");
        switch (choice)
        {
        case 1:
        {
            // 购买商品
            int idx = ConsoleUI::readInt("选择要购买的商品编号: ");
            idx -= 1;
            if (shop.buyItem(idx, player, inventory))
            {
                std::cout << "购买成功！\n";
            }
            else
            {
                std::cout << "购买失败，可能金币不足或编号无效。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            // 出售物品
            if (inventory.isEmpty())
            {
                std::cout << "你的背包里没有可出售的物品。\n";
            }
            else
            {
                inventory.showItems();
                int idx = ConsoleUI::readInt("选择要出售的物品编号: ");
                idx -= 1;
                if (idx >= 0 && idx < inventory.size())
                {
                    std::unique_ptr<Item> soldItem = inventory.sellItem(idx);
                    if (soldItem)
                    {
                        int goldGain = soldItem->getPrice() / 2;
                        player.gainGold(goldGain);
                        std::cout << "已出售 " << soldItem->getName() << "，获得 " << goldGain << " 金币。\n";
                    }
                    else
                    {
                        std::cout << "无法出售该物品。\n";
                    }
                }
                else
                {
                    std::cout << "无效选项，请重新输入。\n";
                }
            }
            ConsoleUI::pause();
            break;
        }
        case 0:
            return;
        default:
            std::cout << "无效选项，请重新输入。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

void GameManager::taskMenu()
{
    while (true)
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("任务列表");
        taskSystem.showTasks();
        std::cout << "\n1. 接受任务\n";
        std::cout << "2. 完成任务\n";
        std::cout << "3. 领取奖励\n";
        std::cout << "0. 返回上一级\n";
        int choice = ConsoleUI::readInt("请选择操作: ");
        switch (choice)
        {
        case 1:
        {
            int idx = ConsoleUI::readInt("输入要接受的任务编号: ");
            idx -= 1;
            if (taskSystem.acceptTask(idx))
            {
                std::cout << "任务已接受！\n";
            }
            else
            {
                std::cout << "无法接受该任务。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            int idx = ConsoleUI::readInt("输入要标记完成的任务编号: ");
            idx -= 1;
            if (taskSystem.completeTask(idx))
            {
                std::cout << "任务已标记为完成！\n";
            }
            else
            {
                std::cout << "无法完成该任务。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 3:
        {
            int idx = ConsoleUI::readInt("输入要领取奖励的任务编号: ");
            idx -= 1;
            if (taskSystem.claimReward(idx, player))
            {
                std::cout << "任务奖励领取成功！\n";
            }
            else
            {
                std::cout << "无法领取该任务的奖励。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 0:
            return;
        default:
            std::cout << "无效选项，请重新输入。\n";
            ConsoleUI::pause();
            break;
        }
    }
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
    std::cout << "\n推荐行动: " << aiAssistant.suggestAction(player, inventory) << "\n";
    ConsoleUI::pause();
}