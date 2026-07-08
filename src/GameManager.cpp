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
    ConsoleUI::printTitle("\xe6\xa0\xa1\xe5\x9b\xad RPG \xe5\x86\x92\xe9\x99\xa9\xe6\xb8\xb8\xe6\x88\x8f");

    shop.loadGoods();
    taskSystem.loadTasks();
    battleSystem.loadEnemies();

    if (!saveManager.loadGame(player, inventory, taskSystem))
    {
        std::string name;
        std::cout << "\xe8\xaf\xb7\xe8\xbe\x93\xe5\x85\xa5\xe4\xbd\xa0\xe7\x9a\x84\xe8\xa7\x92\xe8\x89\xb2\xe5\x90\x8d\xe7\xa7\xb0: ";
        std::getline(std::cin, name);
        player.create(name);

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
        int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe5\x8a\x9f\xe8\x83\xbd: ");
        handleMainMenu(choice);
    }
}

void GameManager::showMainMenu() const
{
    ConsoleUI::printTitle("\xe6\xa0\xa1\xe5\x9b\xad RPG \xe5\x86\x92\xe9\x99\xa9\xe6\xb8\xb8\xe6\x88\x8f");
    std::vector<std::string> menuItems = {
        "1. \xe6\x9f\xa5\xe7\x9c\x8b\xe8\xa7\x92\xe8\x89\xb2\xe4\xbf\xa1\xe6\x81\xaf",
        "2. \xe8\x83\x8c\xe5\x8c\x85\xe7\xae\xa1\xe7\x90\x86",
    };

    if (shopUnlocked)
        menuItems.push_back("3. \xe6\xa0\xa1\xe5\x9b\xad\xe5\x95\x86\xe5\xba\x97");
    else
        menuItems.push_back("3. \xe6\xa0\xa1\xe5\x9b\xad\xe5\x95\x86\xe5\xba\x97  [\xe6\x9c\xaa\xe8\xa7\xa3\xe9\x94\x81]");

    menuItems.push_back("4. \xe4\xbb\xbb\xe5\x8a\xa1\xe7\xb3\xbb\xe7\xbb\x9f");
    menuItems.push_back("5. \xe5\x86\x92\xe9\x99\xa9\xe6\x8c\x91\xe6\x88\x98");
    menuItems.push_back("6. \xe8\x87\xaa\xe7\x94\xb1\xe6\x88\x98\xe6\x96\x97");
    menuItems.push_back("7. AI \xe6\xb8\xb8\xe6\x88\x8f\xe5\x8a\xa9\xe6\x89\x8b");
    menuItems.push_back("8. \xe6\x89\x8b\xe5\x8a\xa8\xe5\xad\x98\xe6\xa1\xa3");
    menuItems.push_back("0. \xe9\x80\x80\xe5\x87\xba\xe6\xb8\xb8\xe6\x88\x8f");

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
            std::cout << "\xe6\xa0\xa1\xe5\x9b\xad\xe5\x95\x86\xe5\xba\x97\xe5\xb0\x9a\xe6\x9c\xaa\xe8\xa7\xa3\xe9\x94\x81\xef\xbc\x81\xe8\xaf\xb7\xe5\x85\x88\xe5\xae\x8c\xe6\x88\x90\xe4\xb8\xbb\xe7\xba\xbf\xe4\xbb\xbb\xe5\x8a\xa1\xe3\x80\x8c\xe8\xae\xa4\xe8\xaf\x86\xe6\x96\xb0\xe4\xb8\x96\xe7\x95\x8c\xe3\x80\x8d\xe3\x80\x82\n";
            ConsoleUI::pause();
        }
        else
        {
            shopMenu();
        }
        break;
    case 4: taskMenu(); break;
    case 5: adventureMenu(); break;
    case 6: battleMenu(); break;
    case 7: aiAssistantMenu(); break;
    case 8:
        saveGame();
        std::cout << "\xe6\xb8\xb8\xe6\x88\x8f\xe5\xb7\xb2\xe4\xbf\x9d\xe5\xad\x98\xe3\x80\x82\n";
        ConsoleUI::pause();
        break;
    case 0: shutdown(); break;
    default:
        std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
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

void GameManager::broadcastTaskEvent(const TaskMessage& msg)
{
    int updated = taskSystem.broadcastMessage(msg);
    if (updated > 0)
        taskSystem.checkAllAutoComplete();
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
        ConsoleUI::printTitle("\xe8\x83\x8c\xe5\x8c\x85\xe7\xae\xa1\xe7\x90\x86");
        if (inventory.isEmpty())
        {
            std::cout << "\xe4\xbd\xa0\xe7\x9a\x84\xe8\x83\x8c\xe5\x8c\x85\xe6\x98\xaf\xe7\xa9\xba\xe7\x9a\x84\xef\xbc\x8c\xe6\xb2\xa1\xe6\x9c\x89\xe4\xbb\xbb\xe4\xbd\x95\xe7\x89\xa9\xe5\x93\x81\xe3\x80\x82\n";
            ConsoleUI::pause();
            return;
        }
        inventory.showItems();
        std::cout << "\n1. \xe4\xbd\xbf\xe7\x94\xa8\xe7\x89\xa9\xe5\x93\x81\n";
        std::cout << "2. \xe4\xb8\xa2\xe5\xbc\x83\xe7\x89\xa9\xe5\x93\x81\n";
        std::cout << "0. \xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\x8a\xe4\xb8\x80\xe7\xba\xa7\n";
        int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe6\x93\x8d\xe4\xbd\x9c: ");
        switch (choice)
        {
        case 1:
        {
            int idx = ConsoleUI::readInt("\xe9\x80\x89\xe6\x8b\xa9\xe8\xa6\x81\xe4\xbd\xbf\xe7\x94\xa8\xe7\x9a\x84\xe7\x89\xa9\xe5\x93\x81\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                if (inventory.useItem(idx, player))
                    std::cout << "\xe7\x89\xa9\xe5\x93\x81\xe4\xbd\xbf\xe7\x94\xa8\xe6\x88\x90\xe5\x8a\x9f\xef\xbc\x81\n";
                else
                    std::cout << "\xe6\x97\xa0\xe6\xb3\x95\xe4\xbd\xbf\xe7\x94\xa8\xe8\xaf\xa5\xe7\x89\xa9\xe5\x93\x81\xe3\x80\x82\n";
            }
            else
            {
                std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            int idx = ConsoleUI::readInt("\xe9\x80\x89\xe6\x8b\xa9\xe8\xa6\x81\xe4\xb8\xa2\xe5\xbc\x83\xe7\x9a\x84\xe7\x89\xa9\xe5\x93\x81\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                std::unique_ptr<Item> soldItem = inventory.sellItem(idx);
                if (soldItem)
                {
                    int goldGain = soldItem->getPrice() / 2;
                    player.gainGold(goldGain);
                    std::cout << "\xe5\xb7\xb2\xe5\x87\xba\xe5\x94\xae " << soldItem->getName() << "\xef\xbc\x8c\xe8\x8e\xb7\xe5\xbe\x97 " << goldGain << " \xe9\x87\x91\xe5\xb8\x81\xe3\x80\x82\n";
                }
            }
            else
            {
                std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 0: return;
        default:
            std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
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
        ConsoleUI::printTitle("\xe6\xa0\xa1\xe5\x9b\xad\xe5\x95\x86\xe5\xba\x97");
        std::cout << "\xe5\xbd\x93\xe5\x89\x8d\xe9\x87\x91\xe5\xb8\x81: " << player.getGold() << "\n\n";
        shop.showGoods();
        std::cout << "\n1. \xe8\xb4\xad\xe4\xb9\xb0\xe7\x89\xa9\xe5\x93\x81\n";
        std::cout << "2. \xe5\x87\xba\xe5\x94\xae\xe7\x89\xa9\xe5\x93\x81\n";
        std::cout << "0. \xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\xbb\xe8\x8f\x9c\xe5\x8d\x95\n";
        int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe6\x93\x8d\xe4\xbd\x9c: ");
        switch (choice)
        {
        case 1:
        {
            int itemIndex = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5\xe8\xa6\x81\xe8\xb4\xad\xe4\xb9\xb0\xe7\x9a\x84\xe7\x89\xa9\xe5\x93\x81\xe7\xbc\x96\xe5\x8f\xb7: ");
            itemIndex -= 1;
            if (shop.buyItem(itemIndex, player, inventory))
            {
                std::cout << "\xe8\xb4\xad\xe4\xb9\xb0\xe6\x88\x90\xe5\x8a\x9f\xef\xbc\x81\n";
            }
            else
            {
                std::cout << "\xe8\xb4\xad\xe4\xb9\xb0\xe5\xa4\xb1\xe8\xb4\xa5\xef\xbc\x8c\xe8\xaf\xb7\xe6\xa3\x80\xe6\x9f\xa5\xe9\x87\x91\xe5\xb8\x81\xe6\x98\xaf\xe5\x90\xa6\xe8\xb6\xb3\xe5\xa4\x9f\xe6\x88\x96\xe7\xbc\x96\xe5\x8f\xb7\xe6\x98\xaf\xe5\x90\xa6\xe6\x9c\x89\xe6\x95\x88\xe3\x80\x82\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            if (inventory.isEmpty())
            {
                std::cout << "\xe8\x83\x8c\xe5\x8c\x85\xe4\xb8\xba\xe7\xa9\xba\xef\xbc\x8c\xe6\xb2\xa1\xe6\x9c\x89\xe7\x89\xa9\xe5\x93\x81\xe5\x8f\xaf\xe4\xbb\xa5\xe5\x87\xba\xe5\x94\xae\xe3\x80\x82\n";
                ConsoleUI::pause();
                break;
            }
            inventory.showItems();
            int idx = ConsoleUI::readInt("\xe9\x80\x89\xe6\x8b\xa9\xe8\xa6\x81\xe5\x87\xba\xe5\x94\xae\xe7\x9a\x84\xe7\x89\xa9\xe5\x93\x81\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            if (idx >= 0 && idx < inventory.size())
            {
                std::unique_ptr<Item> soldItem = inventory.sellItem(idx);
                if (soldItem)
                {
                    int goldGain = soldItem->getPrice() / 2;
                    player.gainGold(goldGain);
                    std::cout << "\xe5\xb7\xb2\xe5\x87\xba\xe5\x94\xae " << soldItem->getName() << "\xef\xbc\x8c\xe8\x8e\xb7\xe5\xbe\x97 " << goldGain << " \xe9\x87\x91\xe5\xb8\x81\xe3\x80\x82\n";
                }
            }
            else
            {
                std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 0: return;
        default:
            std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe8\xaf\xb7\xe9\x87\x8d\xe6\x96\xb0\xe8\xbe\x93\xe5\x85\xa5\xe3\x80\x82\n";
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
        ConsoleUI::printTitle("\xe4\xbb\xbb\xe5\x8a\xa1\xe7\xb3\xbb\xe7\xbb\x9f");

        int mainCount   = taskSystem.countByType(QuestType::Main);
        int worldCount  = taskSystem.countByType(QuestType::World);
        int hiddenCount = taskSystem.countByType(QuestType::Hidden);

        std::cout << "\n";
        ConsoleUI::setColor(GameConfig::COLOR_TITLE);
        std::cout << "  \xe4\xb8\xbb\xe7\xba\xbf\xe4\xbb\xbb\xe5\x8a\xa1: " << mainCount << "  ";
        ConsoleUI::resetColor();
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "\xe4\xb8\x96\xe7\x95\x8c\xe4\xbb\xbb\xe5\x8a\xa1: " << worldCount << "  ";
        ConsoleUI::resetColor();
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "\xe9\x9a\x90\xe8\x97\x8f\xe4\xbb\xbb\xe5\x8a\xa1: " << hiddenCount << "\n";
        ConsoleUI::resetColor();
        std::cout << "\n";

        std::vector<std::string> items = {
            "1. \xe6\x9f\xa5\xe7\x9c\x8b\xe4\xb8\xbb\xe7\xba\xbf\xe4\xbb\xbb\xe5\x8a\xa1",
            "2. \xe6\x9f\xa5\xe7\x9c\x8b\xe4\xb8\x96\xe7\x95\x8c\xe4\xbb\xbb\xe5\x8a\xa1",
            "3. \xe6\x9f\xa5\xe7\x9c\x8b\xe9\x9a\x90\xe8\x97\x8f\xe4\xbb\xbb\xe5\x8a\xa1",
            "4. \xe6\x9f\xa5\xe7\x9c\x8b\xe5\x85\xa8\xe9\x83\xa8\xe4\xbb\xbb\xe5\x8a\xa1",
        };
        ConsoleUI::printBox(items);

        std::cout << "\n0. \xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\xbb\xe8\x8f\x9c\xe5\x8d\x95\n";
        int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9: ");

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
            std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xe3\x80\x82\n";
            ConsoleUI::pause();
            break;
        }
    }
}

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
        std::cout << "===== " << typeLabel << "\xe4\xbb\xbb\xe5\x8a\xa1 =====\n";
        ConsoleUI::resetColor();

        bool showHidden = (type == QuestType::Hidden);
        taskSystem.showTasksByType(type, showHidden);

        std::cout << "\n1. \xe6\x9f\xa5\xe7\x9c\x8b\xe4\xbb\xbb\xe5\x8a\xa1\xe8\xaf\xa6\xe6\x83\x85\n";
        std::cout << "2. \xe6\x8e\xa5\xe5\x8f\x97\xe4\xbb\xbb\xe5\x8a\xa1\n";
        std::cout << "3. \xe9\xa2\x86\xe5\x8f\x96\xe5\xa5\x96\xe5\x8a\xb1\n";
        std::cout << "0. \xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\x8a\xe4\xb8\x80\xe7\xba\xa7\n";

        int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe6\x93\x8d\xe4\xbd\x9c: ");

        switch (choice)
        {
        case 1:
        {
            int idx = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5\xe8\xa6\x81\xe6\x9f\xa5\xe7\x9c\x8b\xe7\x9a\x84\xe4\xbb\xbb\xe5\x8a\xa1\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            taskSystem.showTaskDetail(idx);
            ConsoleUI::pause();
            break;
        }
        case 2:
        {
            int idx = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5\xe8\xa6\x81\xe6\x8e\xa5\xe5\x8f\x97\xe7\x9a\x84\xe4\xbb\xbb\xe5\x8a\xa1\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            if (taskSystem.acceptTask(idx, player))
            {
                std::cout << "\xe4\xbb\xbb\xe5\x8a\xa1\xe5\xb7\xb2\xe6\x8e\xa5\xe5\x8f\x97\xef\xbc\x81\n";
            }
            ConsoleUI::pause();
            break;
        }
        case 3:
        {
            int idx = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5\xe8\xa6\x81\xe9\xa2\x86\xe5\x8f\x96\xe5\xa5\x96\xe5\x8a\xb1\xe7\x9a\x84\xe4\xbb\xbb\xe5\x8a\xa1\xe7\xbc\x96\xe5\x8f\xb7: ");
            idx -= 1;
            if (taskSystem.claimReward(idx, player))
            {
                std::cout << "\xe4\xbb\xbb\xe5\x8a\xa1\xe5\xa5\x96\xe5\x8a\xb1\xe9\xa2\x86\xe5\x8f\x96\xe6\x88\x90\xe5\x8a\x9f\xef\xbc\x81\n";

                if (!shopUnlocked)
                {
                    int main001Idx = taskSystem.findTaskByQuestId("q_main001");
                    if (main001Idx >= 0 &&
                        taskSystem.getTaskStatusByIndex(main001Idx) == TaskStatus::RewardClaimed)
                    {
                        shopUnlocked = true;
                        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
                        std::cout << ">> \xe6\xa0\xa1\xe5\x9b\xad\xe5\x95\x86\xe5\xba\x97\xe5\xb7\xb2\xe8\xa7\xa3\xe9\x94\x81\xef\xbc\x81\xe8\xbf\x94\xe5\x9b\x9e\xe4\xb8\xbb\xe8\x8f\x9c\xe5\x8d\x95\xe5\x8d\xb3\xe5\x8f\xaf\xe8\xae\xbf\xe9\x97\xae\xe3\x80\x82\n";
                        ConsoleUI::resetColor();
                    }
                }
            }
            ConsoleUI::pause();
            break;
        }
        case 0: return;
        default:
            std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xe3\x80\x82\n";
            ConsoleUI::pause();
            break;
        }
    }
}

void GameManager::adventureMenu()
{
    AdventureStatus status = adventureManager.startAdventure(player, inventory);

    if (status == AdventureStatus::SUCCESS)
    {
        broadcastTaskEvent({"adventure_complete", "", 1});
        broadcastTaskEvent({"battle_win", "", 1});
        broadcastTaskEvent({"gold", "", player.getGold()});
    }
    else if (status == AdventureStatus::ESCAPED)
    {
        broadcastTaskEvent({"adventure_escape", "", 1});
    }
}

void GameManager::battleMenu()
{
    BattleResult result = battleSystem.startBattle(player);

    if (result.outcome == BattleOutcome::PLAYER_WIN)
    {
        broadcastTaskEvent({"kill", result.enemyName, 1});
        broadcastTaskEvent({"battle_win", "", 1});
        broadcastTaskEvent({"gold", "", player.getGold()});
    }
}

void GameManager::aiAssistantMenu()
{
    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("AI \xe6\xb8\xb8\xe6\x88\x8f\xe5\x8a\xa9\xe6\x89\x8b");
    aiAssistant.showTips(player);
    std::cout << "\n\xe6\x8e\xa8\xe8\x8d\x90\xe8\xa1\x8c\xe5\x8a\xa8: " << aiAssistant.suggestAction(player, inventory) << "\n";
    ConsoleUI::pause();
}
