// AdventureManager.cpp
#include "AdventureManager.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "Item.h"
#include "PathUtil.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

// ============================================================
// 构造
// ============================================================

AdventureManager::AdventureManager()
    : currentStage(1)
    , maxStages(5)   // 第5场为Boss战
{
}

void AdventureManager::loadEnemyData()
{
    std::string resolved = PathUtil::resolvePath(GameConfig::ENEMIES_FILE_PATH);
    if (resolved.empty()) resolved = GameConfig::ENEMIES_FILE_PATH;
    enemyFactory.loadFromJson(resolved);
}

// ============================================================
// 启动冒险（核心循环）
// ============================================================

AdventureStatus AdventureManager::startAdventure(Character& player, Inventory& inventory)
{
    currentStage = 1;
    temporaryBackpack = {0, {}};

    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("\xe5\xbc\x80\xe5\xa7\x8b\xe6\x96\xb0\xe7\x9a\x84\xe5\x86\x92\xe9\x99\xa9\xef\xbc\x81");
    std::cout << "\xe6\x9c\xac\xe6\xac\xa1\xe5\x86\x92\xe9\x99\xa9\xe5\x85\xb1\xe6\x9c\x89 " << maxStages << " \xe5\x9c\xba\xe6\x88\x98\xe6\x96\x97\xe3\x80\x82\n";
    std::cout << "\xe7\xac\xac 1 \xe5\x9c\xba: 1 \xe6\x99\xae\xe9\x80\x9a\xe6\x80\xaa\n";
    std::cout << "\xe7\xac\xac 2 \xe5\x9c\xba: 2 \xe6\x99\xae\xe9\x80\x9a\xe6\x80\xaa\n";
    std::cout << "\xe7\xac\xac 3 \xe5\x9c\xba: 1 \xe6\x99\xae\xe9\x80\x9a\xe6\x80\xaa + 1 \xe7\xb2\xbe\xe8\x8b\xb1\xe6\x80\xaa\n";
    std::cout << "\xe7\xac\xac 4 \xe5\x9c\xba: 2 \xe7\xb2\xbe\xe8\x8b\xb1\xe6\x80\xaa\n";
    std::cout << "\xe7\xac\xac 5 \xe5\x9c\xba: 1 \xe9\xa6\x96\xe9\xa2\x86 (Boss)\n";
    std::cout << "\n\xe6\xaf\x8f\xe5\x9c\xba\xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\xe5\x8f\xaf\xe8\x8e\xb7\xe5\xbe\x97\xe6\x88\x98\xe5\x88\xa9\xe5\x93\x81\xef\xbc\x8c\xe9\x80\x9a\xe5\x85\xb3\xe5\x90\x8e\xe5\x85\xa8\xe9\x83\xa8\xe5\xb8\xa6\xe5\x9b\x9e\xef\xbc\x81\n";
    std::cout << "\xe4\xb8\xad\xe9\x80\x94\xe9\x80\x83\xe8\xb7\x91\xe4\xbc\x9a\xe9\x9a\x8f\xe6\x9c\xba\xe4\xb8\xa2\xe5\xa4\xb1\xe7\xba\xa6" "30%\xe7\x9a\x84\xe6\x88\x98\xe5\x88\xa9\xe5\x93\x81\xef\xbc\x8c\xe6\x88\x98\xe8\xb4\xa5\xe5\x88\x99\xe5\x85\xa8\xe9\x83\xa8\xe4\xb8\xa2\xe5\xa4\xb1\xe3\x80\x82\n";
    ConsoleUI::pause();

    while (currentStage <= maxStages)
    {
        // 使用工厂根据当前阶段生成敌人
        std::vector<Enemy> stageEnemies = enemyFactory.generateEnemies(currentStage);
        bool isBoss = (currentStage == maxStages);

        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("\xe5\x86\x92\xe9\x99\xa9\xe8\xbf\x9b\xe8\xa1\x8c\xe4\xb8\xad");
        std::cout << "\xe5\xbd\x93\xe5\x89\x8d\xe8\xbf\x9b\xe5\xba\xa6: \xe7\xac\xac " << currentStage << " / " << maxStages << " \xe5\x9c\xba\n";
        std::cout << "\xe4\xb8\xb4\xe6\x97\xb6\xe8\x83\x8c\xe5\x8c\x85: " << temporaryBackpack.gold << " \xe9\x87\x91\xe5\xb8\x81";
        if (!temporaryBackpack.items.empty())
        {
            std::cout << ", " << temporaryBackpack.items.size() << " \xe4\xbb\xb6\xe7\x89\xa9\xe5\x93\x81";
        }
        std::cout << "\n\n";

        ConsoleUI::printLine('-');
        if (isBoss)
        {
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "!!! Boss \xe6\x88\x98 !!!  \xe6\x95\x8c\xe4\xba\xba\xe6\x95\xb0\xe9\x87\x8f: " << stageEnemies.size() << "\n";
            ConsoleUI::resetColor();
        }
        else
        {
            std::cout << "\xe7\xac\xac " << currentStage << " \xe5\x9c\xba\xe6\x88\x98\xe6\x96\x97  \xe6\x95\x8c\xe4\xba\xba\xe6\x95\xb0\xe9\x87\x8f: " << stageEnemies.size() << "\n";
        }

        // 显示敌人信息
        for (size_t i = 0; i < stageEnemies.size(); ++i)
        {
            std::cout << "  " << (i + 1) << ". ";
            stageEnemies[i].showInfo();
        }

        BattleOutcome result = triggerBattle(stageEnemies);

        if (result == BattleOutcome::PLAYER_WIN)
        {
            // 胜利：根据敌人生成战利品
            Loot dropped = generateLoot(stageEnemies);
            temporaryBackpack.gold += dropped.gold;

            ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
            std::cout << "\n\xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\xef\xbc\x81\xe8\x8e\xb7\xe5\xbe\x97 " << dropped.gold << " \xe9\x87\x91\xe5\xb8\x81";
            if (!dropped.items.empty())
            {
                std::cout << " \xe5\x92\x8c " << dropped.items.size() << " \xe4\xbb\xb6\xe7\x89\xa9\xe5\x93\x81:";
                for (const auto& item : dropped.items)
                    std::cout << " [" << item << "]";
            }
            std::cout << "\n";
            ConsoleUI::resetColor();

            for (const auto& item : dropped.items)
                temporaryBackpack.items.push_back(item);

            currentStage++;
            ConsoleUI::pause();
        }
        else if (result == BattleOutcome::PLAYER_ESCAPE)
        {
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "\n\xe4\xbd\xa0\xe9\x80\x89\xe6\x8b\xa9\xe4\xba\x86\xe9\x80\x83\xe8\xb7\x91\xef\xbc\x81\xe5\xb0\x86\xe4\xb8\xa2\xe5\xa4\xb1\xe7\xba\xa6" "30%\xe7\x9a\x84\xe6\x88\x98\xe5\x88\xa9\xe5\x93\x81...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::ESCAPED, player, inventory);
            return AdventureStatus::ESCAPED;
        }
        else // PLAYER_DEAD
        {
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "\n\xe4\xbd\xa0\xe8\xa2\xab\xe5\x87\xbb\xe8\xb4\xa5\xe4\xba\x86\xef\xbc\x81\xe6\x89\x80\xe6\x9c\x89\xe4\xb8\xb4\xe6\x97\xb6\xe6\x88\x98\xe5\x88\xa9\xe5\x93\x81\xe5\x85\xa8\xe9\x83\xa8\xe4\xb8\xa2\xe5\xa4\xb1...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::DEFEATED, player, inventory);
            return AdventureStatus::DEFEATED;
        }
    }

    // 通关结算
    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "\n\xe6\x81\xad\xe5\x96\x9c\xef\xbc\x81\xe4\xbd\xa0\xe6\x88\x90\xe5\x8a\x9f\xe9\x80\x9a\xe5\x85\xb3\xe4\xba\x86\xe6\x9c\xac\xe6\xac\xa1\xe5\x86\x92\xe9\x99\xa9\xef\xbc\x81\n";
    std::cout << "\xe6\x89\x80\xe6\x9c\x89\xe6\x88\x98\xe5\x88\xa9\xe5\x93\x81\xe5\xb7\xb2\xe5\xae\x89\xe5\x85\xa8\xe5\xb8\xa6\xe5\x9b\x9e\xe3\x80\x82\n";
    ConsoleUI::resetColor();
    settleAdventure(AdventureStatus::SUCCESS, player, inventory);
    return AdventureStatus::SUCCESS;
}

// ============================================================
// 占位战斗接口
// 显示敌人信息，使用控制台输入模拟战斗结果
// ============================================================

BattleOutcome AdventureManager::triggerBattle(const std::vector<Enemy>& enemies)
{
    (void)enemies; // 当前占位，后续真实战斗会使用

    std::cout << "\n--- \xe6\x88\x98\xe6\x96\x97\xe6\xa8\xa1\xe6\x8b\x9f ---\n";
    std::cout << "1. \xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\n";
    std::cout << "2. \xe9\x80\x83\xe8\xb7\x91\n";
    std::cout << "3. \xe6\x88\x98\xe8\xb4\xa5\n";

    int choice = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe6\x88\x98\xe6\x96\x97\xe7\xbb\x93\xe6\x9e\x9c: ");

    switch (choice)
    {
    case 1:  return BattleOutcome::PLAYER_WIN;
    case 2:  return BattleOutcome::PLAYER_ESCAPE;
    case 3:  return BattleOutcome::PLAYER_DEAD;
    default:
        std::cout << "\xe6\x97\xa0\xe6\x95\x88\xe9\x80\x89\xe9\xa1\xb9\xef\xbc\x8c\xe9\xbb\x98\xe8\xae\xa4\xe5\x88\xa4\xe5\xae\x9a\xe4\xb8\xba\xe8\x83\x9c\xe5\x88\xa9\xe3\x80\x82\n";
        return BattleOutcome::PLAYER_WIN;
    }
}

// ============================================================
// 战利品生成（基于敌人的lootItems）
// ============================================================

Loot AdventureManager::generateLoot(const std::vector<Enemy>& enemies)
{
    Loot loot;

    for (const auto& enemy : enemies)
    {
        // 金币 = 敌人奖励金 + 随机加成
        int bonus = rand() % (enemy.getRewardGold() / 2 + 1);
        loot.gold += enemy.getRewardGold() + bonus;

        // 根据敌人的lootItems掉落物品
        const auto& enemyLoot = enemy.getLootItems();
        if (!enemyLoot.empty())
        {
            // 按分级决定掉落数量
            int dropCount = 1;
            switch (enemy.getTier())
            {
            case EnemyTier::Minion: dropCount = 1; break;
            case EnemyTier::Elite:  dropCount = (rand() % 2) + 1; break;  // 1-2
            case EnemyTier::Boss:   dropCount = (rand() % 2) + 2; break;  // 2-3
            }

            for (int i = 0; i < dropCount; ++i)
            {
                int idx = rand() % enemyLoot.size();
                loot.items.push_back(enemyLoot[idx]);
            }
        }
    }

    return loot;
}

// ============================================================
// 冒险结算
// ============================================================

void AdventureManager::settleAdventure(AdventureStatus status, Character& player, Inventory& inventory)
{
    ConsoleUI::printLine('=');
    ConsoleUI::printTitle("\xe5\x86\x92\xe9\x99\xa9\xe7\xbb\x93\xe7\xae\x97");

    switch (status)
    {
    case AdventureStatus::SUCCESS:
    {
        std::cout << "\xe7\xbb\x93\xe7\xae\x97\xe7\xbb\x93\xe6\x9e\x9c: \xe6\x88\x90\xe5\x8a\x9f\xe9\x80\x9a\xe5\x85\xb3\xef\xbc\x81\n";
        std::cout << "\xe8\x8e\xb7\xe5\xbe\x97\xe9\x87\x91\xe5\xb8\x81: " << temporaryBackpack.gold << "\n";
        std::cout << "\xe8\x8e\xb7\xe5\xbe\x97\xe7\x89\xa9\xe5\x93\x81: " << temporaryBackpack.items.size() << " \xe4\xbb\xb6\n";

        player.gainGold(temporaryBackpack.gold);
        for (const auto& itemName : temporaryBackpack.items)
            addItemToInventory(itemName, inventory);
        break;
    }

    case AdventureStatus::ESCAPED:
    {
        int totalItems = static_cast<int>(temporaryBackpack.items.size());
        int loseCount = std::max(1, totalItems * 30 / 100);

        std::vector<int> indices(totalItems);
        for (int i = 0; i < totalItems; ++i) indices[i] = i;

        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::shuffle(indices.begin(), indices.end(), rng);

        std::vector<std::string> lostItems;
        for (int i = 0; i < loseCount && i < totalItems; ++i)
            lostItems.push_back(temporaryBackpack.items[indices[i]]);

        std::cout << "\xe7\xbb\x93\xe7\xae\x97\xe7\xbb\x93\xe6\x9e\x9c: \xe4\xb8\xad\xe9\x80\x94\xe9\x80\x83\xe8\xb7\x91\n";
        std::cout << "\xe4\xb8\xa2\xe5\xa4\xb1\xe9\x87\x91\xe5\xb8\x81: " << temporaryBackpack.gold << "\xef\xbc\x88\xe9\x80\x83\xe8\xb7\x91\xe9\x87\x91\xe5\xb8\x81\xe5\x85\xa8\xe4\xb8\xa2\xef\xbc\x89\n";
        std::cout << "\xe4\xb8\xa2\xe5\xa4\xb1\xe7\x89\xa9\xe5\x93\x81: ";
        for (const auto& name : lostItems)
            std::cout << "[" << name << "] ";
        std::cout << "\n";

        std::vector<bool> lost(totalItems, false);
        for (int i = 0; i < loseCount && i < totalItems; ++i)
            lost[indices[i]] = true;

        int keptCount = 0;
        for (int i = 0; i < totalItems; ++i)
        {
            if (!lost[i])
            {
                addItemToInventory(temporaryBackpack.items[i], inventory);
                keptCount++;
            }
        }
        std::cout << "\xe4\xbf\x9d\xe7\x95\x99\xe7\x89\xa9\xe5\x93\x81: " << keptCount << " \xe4\xbb\xb6\n";
        break;
    }

    case AdventureStatus::DEFEATED:
    {
        std::cout << "\xe7\xbb\x93\xe7\xae\x97\xe7\xbb\x93\xe6\x9e\x9c: \xe6\x88\x98\xe8\xb4\xa5\n";
        std::cout << "\xe4\xb8\xa2\xe5\xa4\xb1\xe9\x87\x91\xe5\xb8\x81: " << temporaryBackpack.gold << "\n";
        std::cout << "\xe4\xb8\xa2\xe5\xa4\xb1\xe7\x89\xa9\xe5\x93\x81: " << temporaryBackpack.items.size() << " \xe4\xbb\xb6\n";
        std::cout << "\xe4\xbf\x9d\xe7\x95\x99\xe7\x89\xa9\xe5\x93\x81: 0 \xe4\xbb\xb6\n";
        break;
    }

    default:
        break;
    }

    temporaryBackpack = {0, {}};
    ConsoleUI::pause();
}

// ============================================================
// 将物品名称转换为 Item 并加入背包
// ============================================================

void AdventureManager::addItemToInventory(const std::string& itemName, Inventory& inventory)
{
    if (itemName == "\xe9\x9d\xa2\xe5\x8c\x85")
    {
        inventory.addItem(std::make_unique<FoodItem>("\xe9\x9d\xa2\xe5\x8c\x85", "\xe6\x99\xae\xe9\x80\x9a\xe9\x9d\xa2\xe5\x8c\x85\xef\xbc\x8c\xe6\x81\xa2\xe5\xa4\x8d\xe5\xb0\x91\xe9\x87\x8f\xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc", 10, 15));
    }
    else if (itemName == "\xe8\x8d\xaf\xe8\x8d\x89")
    {
        inventory.addItem(std::make_unique<MedicineItem>("\xe8\x8d\xaf\xe8\x8d\x89", "\xe8\x8d\x89\xe8\x8d\xaf\xef\xbc\x8c\xe6\x81\xa2\xe5\xa4\x8d\xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc", 15, 25));
    }
    else if (itemName == "\xe7\xbb\xb7\xe5\xb8\xa6")
    {
        inventory.addItem(std::make_unique<MedicineItem>("\xe7\xbb\xb7\xe5\xb8\xa6", "\xe6\x80\xa5\xe6\x95\x91\xe7\xbb\xb7\xe5\xb8\xa6\xef\xbc\x8c\xe6\x81\xa2\xe5\xa4\x8d\xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc", 20, 35));
    }
    else if (itemName == "\xe9\x93\x81\xe5\x89\x91")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("\xe9\x93\x81\xe5\x89\x91", "\xe9\x94\x8b\xe5\x88\xa9\xe7\x9a\x84\xe9\x93\x81\xe5\x89\x91", 50, 8, 0));
    }
    else if (itemName == "\xe7\x9a\xae\xe7\x94\xb2")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("\xe7\x9a\xae\xe7\x94\xb2", "\xe8\xbd\xbb\xe4\xbe\xbf\xe7\x9a\x84\xe7\x9a\xae\xe7\x94\xb2", 50, 0, 5));
    }
    else if (itemName == "\xe5\x8a\x9b\xe9\x87\x8f\xe6\x88\x92\xe6\x8c\x87")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("\xe5\x8a\x9b\xe9\x87\x8f\xe6\x88\x92\xe6\x8c\x87", "\xe5\xa2\x9e\xe5\x8a\xa0\xe6\x94\xbb\xe5\x87\xbb\xe5\x8a\x9b\xe7\x9a\x84\xe6\x88\x92\xe6\x8c\x87", 60, 5, 2));
    }
    else if (itemName == "\xe6\xb2\xbb\xe7\x96\x97\xe8\x8d\xaf\xe6\xb0\xb4")
    {
        inventory.addItem(std::make_unique<MedicineItem>("\xe6\xb2\xbb\xe7\x96\x97\xe8\x8d\xaf\xe6\xb0\xb4", "\xe5\xbc\xba\xe5\x8a\x9b\xe6\xb2\xbb\xe7\x96\x97\xe8\x8d\xaf\xe6\xb0\xb4", 40, 50));
    }
}
