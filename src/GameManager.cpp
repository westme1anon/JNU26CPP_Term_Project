// GameManager.cpp
#include "GameManager.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>
#include <string>

GameManager::GameManager()
    : running(true), shopUnlocked(false)
{
}

void GameManager::init()
{
    ConsoleUI::setupConsole();
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("校园 RPG 冒险游戏");

    shop.loadGoods();
    taskSystem.loadTasks();
    battleSystem.loadEnemies();

    if (!saveManager.loadGame(player, inventory, taskSystem))
    {
        std::string name;
        std::cout << "请输入你的角色名称: ";
        std::getline(std::cin, name);
        player.create(name);

        // 新角色：自动接取默认任务
        taskSystem.autoAcceptDefaults();
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
    std::vector<std::string> menuItems = {
        "1. 查看角色信息",
        "2. 背包管理",
    };

    if (shopUnlocked)
        menuItems.push_back("3. 校园商店");
    else
        menuItems.push_back("3. 校园商店  [未解锁]");

    menuItems.push_back("4. 任务系统");
    menuItems.push_back("5. 战斗挑战");
    menuItems.push_back("6. AI 游戏助手");
    menuItems.push_back("7. 手动存档");
    menuItems.push_back("0. 退出游戏");

    ConsoleUI::printBox(menuItems);
}

void GameManager::handleMainMenu(int choice)
{
    switch (choice)
    {
    case 1: characterMenu(); break;
    case 2: inventoryMenu(); break;
    case 3:
        if (!shopUnlocked)
        {
            std::cout << "校园商店尚未解锁！请先完成主线任务「认识新世界」。\n";
            ConsoleUI::pause();
        }
        else
        {
            shopMenu();
        }
        break;
    case 4: taskMenu(); break;
    case 5: battleMenu(); break;
    case 6: aiAssistantMenu(); break;
    case 7:
        saveGame();
        std::cout << "游戏已保存。\n";
        ConsoleUI::pause();
        break;
    case 0: shutdown(); break;
    default:
        std::cout << "无效选项，请重新输入。\n";
        ConsoleUI::pause();
        break;
    }
}

bool GameManager::saveGame()
{
    return saveManager.saveGame(player, inventory, taskSystem);
}

void GameManager::shutdown()
{
    autoSaveService.stop();
    saveGame();
    running = false;
}

// ============================================================
// 辅助
// ============================================================

void GameManager::broadcastTaskEvent(const TaskMessage& msg)
{
    int updated = taskSystem.broadcastMessage(msg);
    if (updated > 0)
        taskSystem.checkAllAutoComplete();
}

// ============================================================
// 角色菜单
// ============================================================

void GameManager::characterMenu()
{
    ConsoleUI::clearScreen();
    player.showInfo();
    ConsoleUI::pause();
}

// ============================================================
// 背包菜单
// ============================================================

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
            int idx = ConsoleUI::readInt("选择要使用的物品编号: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                if (inventory.useItem(idx, player))
                    std::cout << "物品使用成功！\n";
                else
                    std::cout << "无法使用该物品。\n";
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
                    broadcastTaskEvent({"gold", "", player.getGold()});
                }
            }
            else
            {
                std::cout << "无效选项，请重新输入。\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 0: return;
        default:
            std::cout << "无效选项，请重新输入。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

// ============================================================
// 商店菜单
// ============================================================

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
            int idx = ConsoleUI::readInt("选择要购买的商品编号: ");
            idx -= 1;
            if (shop.buyItem(idx, player, inventory))
            {
                std::cout << "购买成功！\n";
                broadcastTaskEvent({"collect", "装备", 1});
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
                        broadcastTaskEvent({"gold", "", player.getGold()});
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
        case 0: return;
        default:
            std::cout << "无效选项，请重新输入。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

// ============================================================
// 任务系统主菜单（三级布局）
// ============================================================

void GameManager::taskMenu()
{
    while (true)
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("任务系统");

        // 分类统计
        int mainCount = taskSystem.countByType(QuestType::Main);
        int worldCount = taskSystem.countByType(QuestType::World);
        int hiddenCount = taskSystem.countByType(QuestType::Hidden);

        std::cout << "\n";
        ConsoleUI::setColor(GameConfig::COLOR_TITLE);
        std::cout << "  主线任务: " << mainCount << "  ";
        ConsoleUI::resetColor();
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "世界任务: " << worldCount << "  ";
        ConsoleUI::resetColor();
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "隐藏任务: " << hiddenCount << "\n";
        ConsoleUI::resetColor();
        std::cout << "\n";

        std::vector<std::string> items = {
            "1. 查看主线任务",
            "2. 查看世界任务",
            "3. 查看隐藏任务",
            "4. 查看全部任务",
        };
        ConsoleUI::printBox(items);

        std::cout << "\n0. 返回主菜单\n";
        int choice = ConsoleUI::readInt("请选择: ");

        switch (choice)
        {
        case 1: taskSubMenu(QuestType::Main); break;
        case 2: taskSubMenu(QuestType::World); break;
        case 3: taskSubMenu(QuestType::Hidden); break;
        case 4:
            ConsoleUI::clearScreen();
            taskSystem.showTasksOverview();
            ConsoleUI::pause();
            break;
        case 0: return;
        default:
            std::cout << "无效选项。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

// ============================================================
// 任务二级菜单（按类型）
// ============================================================

void GameManager::taskSubMenu(QuestType type)
{
    while (true)
    {
        ConsoleUI::clearScreen();

        const char* typeLabel = questTypeToString(type);
        int color = (type == QuestType::Main) ? GameConfig::COLOR_TITLE :
                    (type == QuestType::World) ? GameConfig::COLOR_SUCCESS :
                                                 GameConfig::COLOR_WARNING;

        ConsoleUI::setColor(color);
        std::cout << "===== " << typeLabel << "任务 =====\n";
        ConsoleUI::resetColor();

        bool showHidden = (type == QuestType::Hidden);
        taskSystem.showTasksByType(type, showHidden);

        std::cout << "\n1. 查看任务详情\n";
        std::cout << "2. 接受任务\n";
        std::cout << "3. 领取奖励\n";
        std::cout << "0. 返回上一级\n";

        int choice = ConsoleUI::readInt("请选择操作: ");

        switch (choice)
        {
        case 1:
        {
            int idx = ConsoleUI::readInt("输入要查看的任务编号: ");
            idx -= 1;
            taskSystem.showTaskDetail(idx);
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            int idx = ConsoleUI::readInt("输入要接受的任务编号: ");
            idx -= 1;
            if (taskSystem.acceptTask(idx, player))
            {
                std::cout << "任务已接受！\n";
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

                // 如果 q_main001 已完成领取，解锁商店
                if (!shopUnlocked)
                {
                    int main001Idx = taskSystem.findTaskByQuestId("q_main001");
                    if (main001Idx >= 0 &&
                        taskSystem.getTaskStatusByIndex(main001Idx) == TaskStatus::RewardClaimed)
                    {
                        shopUnlocked = true;
                        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
                        std::cout << ">> 校园商店已解锁！返回主菜单即可访问。\n";
                        ConsoleUI::resetColor();
                    }
                }
            }
            ConsoleUI::pause();
            break;
        }
        case 0: return;
        default:
            std::cout << "无效选项。\n";
            ConsoleUI::pause();
            break;
        }
    }
}

// ============================================================
// 战斗菜单
// ============================================================

void GameManager::battleMenu()
{
    BattleResult result = battleSystem.startBattle(player);

    if (result.playerWon)
    {
        broadcastTaskEvent({"kill", result.enemyName, 1});
        broadcastTaskEvent({"battle_win", "", 1});
        broadcastTaskEvent({"gold", "", player.getGold()});
    }
}

// ============================================================
// AI 助手菜单
// ============================================================

void GameManager::aiAssistantMenu()
{
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("AI 游戏助手");
    aiAssistant.showTips(player);
    std::cout << "\n推荐行动: " << aiAssistant.suggestAction(player, inventory) << "\n";
    ConsoleUI::pause();
}
