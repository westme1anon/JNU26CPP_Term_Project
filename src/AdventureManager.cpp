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

AdventureManager::AdventureManager()
    : currentStage(1), maxStages(5)
{
}

void AdventureManager::loadEnemyData()
{
    std::string resolved = PathUtil::resolvePath(GameConfig::ENEMIES_FILE_PATH);
    if (resolved.empty()) resolved = GameConfig::ENEMIES_FILE_PATH;
    enemyFactory.loadFromJson(resolved);
    // AdventureManager 自己需要敌人列表做展示和掉落计算，
    // BattleSystem 也需要同一份敌人数据做正式战斗，因此两边都加载。
    battleSystem.loadEnemies();
}

AdventureStatus AdventureManager::startAdventure(Character& player, Inventory& inventory)
{
    currentStage = 1;
    temporaryBackpack = {0, {}};

    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("开始新的冒险！");
    std::cout << "本次冒险共有 " << maxStages << " 场战斗。\n";
    std::cout << "第 1 场: 1 普通怪\n";
    std::cout << "第 2 场: 2 普通怪\n";
    std::cout << "第 3 场: 1 普通怪 + 1 精英怪\n";
    std::cout << "第 4 场: 2 精英怪\n";
    std::cout << "第 5 场: 1 首领 (Boss)\n";
    std::cout << "\n每场战斗胜利可获得战利品，通关后全部带回！\n";
    std::cout << "中途逃跑会随机丢失约30%的战利品，战败则全部丢失。\n";
    ConsoleUI::pause();

    while (currentStage <= maxStages)
    {
        std::vector<Enemy> stageEnemies = enemyFactory.generateEnemies(currentStage);
        const bool isBoss = (currentStage == maxStages);

        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("冒险进行中");
        std::cout << "当前进度: 第 " << currentStage << " / " << maxStages << " 场\n";
        std::cout << "临时背包: " << temporaryBackpack.gold << " 金币";
        if (!temporaryBackpack.items.empty())
            std::cout << ", " << temporaryBackpack.items.size() << " 件物品";
        std::cout << "\n\n";

        ConsoleUI::printLine('-');
        if (isBoss)
        {
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "!!! Boss 战 !!!  敌人数量: " << stageEnemies.size() << "\n";
            ConsoleUI::resetColor();
        }
        else
        {
            std::cout << "第 " << currentStage << " 场战斗  敌人数量: " << stageEnemies.size() << "\n";
        }

        for (size_t i = 0; i < stageEnemies.size(); ++i)
        {
            std::cout << "  " << (i + 1) << ". ";
            stageEnemies[i].showInfo();
        }

        const BattleOutcome result = triggerBattle(player, inventory, stageEnemies);
        if (result == BattleOutcome::PLAYER_WIN)
        {
            Loot dropped = generateLoot(stageEnemies);
            temporaryBackpack.gold += dropped.gold;

            ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
            std::cout << "\n战斗胜利！获得 " << dropped.gold << " 金币";
            if (!dropped.items.empty())
            {
                std::cout << " 和 " << dropped.items.size() << " 件物品:";
                for (const auto& item : dropped.items)
                    std::cout << " [" << item << "]";
            }
            std::cout << "\n";
            ConsoleUI::resetColor();

            for (const auto& item : dropped.items)
                temporaryBackpack.items.push_back(item);

            ++currentStage;
            ConsoleUI::pause();
        }
        else if (result == BattleOutcome::PLAYER_ESCAPE)
        {
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "\n你选择了逃跑！将丢失约30%的战利品...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::ESCAPED, player, inventory);
            return AdventureStatus::ESCAPED;
        }
        else
        {
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "\n你被击败了！所有临时战利品全部丢失...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::DEFEATED, player, inventory);
            return AdventureStatus::DEFEATED;
        }
    }

    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "\n恭喜！你成功通关了本次冒险！\n";
    std::cout << "所有战利品已安全带回。\n";
    ConsoleUI::resetColor();
    settleAdventure(AdventureStatus::SUCCESS, player, inventory);
    return AdventureStatus::SUCCESS;
}

BattleOutcome AdventureManager::triggerBattle(Character& player, Inventory& inventory, const std::vector<Enemy>& enemies)
{
    const bool isBoss = (currentStage == maxStages);
    // 直接把当前阶段已经生成好的敌人列表交给 BattleSystem，
    // 避免再次随机生成造成“看到的敌人”和“实际打的敌人”不一致。
    BattleResult result = battleSystem.startBattle(player, inventory, enemies, isBoss);
    return result.outcome;
}

Loot AdventureManager::generateLoot(const std::vector<Enemy>& enemies)
{
    Loot loot;

    for (const auto& enemy : enemies)
    {
        const int bonus = rand() % (enemy.getRewardGold() / 2 + 1);
        loot.gold += enemy.getRewardGold() + bonus;

        const auto& enemyLoot = enemy.getLootItems();
        if (enemyLoot.empty())
            continue;

        int dropCount = 1;
        // 掉落数量继续沿用原冒险系统的 tier 逻辑，
        // 和新卡牌战斗本身解耦，便于后续单独调平衡。
        switch (enemy.getTier())
        {
        case EnemyTier::Minion: dropCount = 1; break;
        case EnemyTier::Elite: dropCount = (rand() % 2) + 1; break;
        case EnemyTier::Boss: dropCount = (rand() % 2) + 2; break;
        }

        for (int i = 0; i < dropCount; ++i)
            loot.items.push_back(enemyLoot[rand() % enemyLoot.size()]);
    }

    return loot;
}

void AdventureManager::settleAdventure(AdventureStatus status, Character& player, Inventory& inventory)
{
    ConsoleUI::printLine('=');
    ConsoleUI::printTitle("冒险结算");

    switch (status)
    {
    case AdventureStatus::SUCCESS:
        std::cout << "结算结果: 成功通关！\n";
        std::cout << "获得金币: " << temporaryBackpack.gold << "\n";
        std::cout << "获得物品: " << temporaryBackpack.items.size() << " 件\n";
        player.gainGold(temporaryBackpack.gold);
        for (const auto& itemName : temporaryBackpack.items)
            addItemToInventory(itemName, inventory);
        break;

    case AdventureStatus::ESCAPED:
    {
        const int totalItems = static_cast<int>(temporaryBackpack.items.size());
        const int loseCount = totalItems > 0 ? std::max(1, totalItems * 30 / 100) : 0;

        std::vector<int> indices(totalItems);
        for (int i = 0; i < totalItems; ++i) indices[i] = i;

        static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::shuffle(indices.begin(), indices.end(), rng);

        std::vector<std::string> lostItems;
        for (int i = 0; i < loseCount && i < totalItems; ++i)
            lostItems.push_back(temporaryBackpack.items[indices[i]]);

        std::cout << "结算结果: 中途逃跑\n";
        std::cout << "丢失金币: " << temporaryBackpack.gold << "（逃跑金币全丢）\n";
        std::cout << "丢失物品: ";
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
                ++keptCount;
            }
        }
        std::cout << "保留物品: " << keptCount << " 件\n";
        break;
    }

    case AdventureStatus::DEFEATED:
        std::cout << "结算结果: 战败\n";
        std::cout << "丢失金币: " << temporaryBackpack.gold << "\n";
        std::cout << "丢失物品: " << temporaryBackpack.items.size() << " 件\n";
        std::cout << "保留物品: 0 件\n";
        break;

    default:
        break;
    }

    temporaryBackpack = {0, {}};
    ConsoleUI::pause();
}

void AdventureManager::addItemToInventory(const std::string& itemName, Inventory& inventory)
{
    if (itemName == "面包")
    {
        inventory.addItem(std::make_unique<FoodItem>("面包", "普通面包，恢复少量生命值", 10, 15));
    }
    else if (itemName == "药草")
    {
        inventory.addItem(std::make_unique<MedicineItem>("药草", "草药，恢复生命值", 15, 25));
    }
    else if (itemName == "绷带")
    {
        inventory.addItem(std::make_unique<MedicineItem>("绷带", "急救绷带，恢复生命值", 20, 35));
    }
    else if (itemName == "铁剑")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("铁剑", "锋利的铁剑", 50, 8, 0));
    }
    else if (itemName == "皮甲")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("皮甲", "轻便的皮甲", 50, 0, 5));
    }
    else if (itemName == "力量戒指")
    {
        inventory.addItem(std::make_unique<EquipmentItem>("力量戒指", "增加攻击力的戒指", 60, 5, 2));
    }
    else if (itemName == "治疗药水")
    {
        inventory.addItem(std::make_unique<MedicineItem>("治疗药水", "强力治疗药水", 40, 50));
    }
}
