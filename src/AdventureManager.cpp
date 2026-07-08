// AdventureManager.cpp
#include "AdventureManager.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "Item.h"

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

// ============================================================
// 启动冒险（核心循环）
// ============================================================

AdventureStatus AdventureManager::startAdventure(Character& player, Inventory& inventory)
{
    currentStage = 1;
    temporaryBackpack = {0, {}};

    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("开始新的冒险！");
    std::cout << "本次冒险共有 " << maxStages << " 场战斗，第 " << maxStages << " 场为 Boss 战。\n";
    std::cout << "每场战斗胜利可获得战利品，通关后全部带回！\n";
    std::cout << "中途逃跑会随机丢失约30%的战利品，战败则全部丢失。\n";
    ConsoleUI::pause();

    while (currentStage <= maxStages)
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("冒险进行中");
        std::cout << "当前进度: 第 " << currentStage << " / " << maxStages << " 场\n";
        std::cout << "临时背包: " << temporaryBackpack.gold << " 金币";
        if (!temporaryBackpack.items.empty())
        {
            std::cout << ", " << temporaryBackpack.items.size() << " 件物品";
        }
        std::cout << "\n\n";

        // 根据关卡计算怪物等级
        int monsterLevel = currentStage * 2;
        bool isBoss = (currentStage == maxStages);

        ConsoleUI::printLine('-');
        if (isBoss)
        {
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "!!! Boss 战 !!!  怪物等级: " << monsterLevel << "\n";
            ConsoleUI::resetColor();
        }
        else
        {
            std::cout << "第 " << currentStage << " 场战斗  怪物等级: " << monsterLevel << "\n";
        }

        BattleOutcome result = triggerBattle(monsterLevel, isBoss);

        if (result == BattleOutcome::PLAYER_WIN)
        {
            // 胜利：获得战利品，继续下一关
            Loot dropped = generateLoot(monsterLevel);
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

            currentStage++;
            ConsoleUI::pause();
        }
        else if (result == BattleOutcome::PLAYER_ESCAPE)
        {
            // 逃跑结算：扣除约30%战利品，结束冒险
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "\n你选择了逃跑！将丢失约30%的战利品...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::ESCAPED, player, inventory);
            return AdventureStatus::ESCAPED;
        }
        else // PLAYER_DEAD
        {
            // 战败结算：丢失全部战利品
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "\n你被击败了！所有临时战利品全部丢失...\n";
            ConsoleUI::resetColor();
            settleAdventure(AdventureStatus::DEFEATED, player, inventory);
            return AdventureStatus::DEFEATED;
        }
    }

    // 通关结算
    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "\n恭喜！你成功通关了本次冒险！\n";
    std::cout << "所有战利品已安全带回。\n";
    ConsoleUI::resetColor();
    settleAdventure(AdventureStatus::SUCCESS, player, inventory);
    return AdventureStatus::SUCCESS;
}

// ============================================================
// 占位战斗接口
// 使用控制台输入模拟战斗结果：1-胜利 2-逃跑 3-战败
// ============================================================

BattleOutcome AdventureManager::triggerBattle(int monsterLevel, bool isBoss)
{
    (void)monsterLevel; // 当前占位，后续真实战斗会使用
    (void)isBoss;

    std::cout << "\n--- 战斗模拟 ---\n";
    std::cout << "1. 战斗胜利\n";
    std::cout << "2. 逃跑\n";
    std::cout << "3. 战败\n";

    int choice = ConsoleUI::readInt("请选择战斗结果: ");

    switch (choice)
    {
    case 1:  return BattleOutcome::PLAYER_WIN;
    case 2:  return BattleOutcome::PLAYER_ESCAPE;
    case 3:  return BattleOutcome::PLAYER_DEAD;
    default:
        std::cout << "无效选项，默认判定为胜利。\n";
        return BattleOutcome::PLAYER_WIN;
    }
}

// ============================================================
// 战利品生成
// ============================================================

Loot AdventureManager::generateLoot(int monsterLevel)
{
    Loot loot;

    // 基础金币 = 等级 x 20 + 随机加成
    loot.gold = monsterLevel * 20 + (rand() % (monsterLevel * 5 + 1));

    // 根据等级随机掉落物品
    int itemCount = (monsterLevel >= 8) ? 2 : (monsterLevel >= 4 ? 1 : 0);
    if (rand() % 100 < 60 + monsterLevel * 3) // 随等级提高掉落概率
        itemCount = std::max(itemCount, 1);

    static const std::vector<std::string> commonItems = {
        "面包", "药草", "绷带"
    };
    static const std::vector<std::string> rareItems = {
        "铁剑", "皮甲", "力量戒指", "治疗药水"
    };

    for (int i = 0; i < itemCount; ++i)
    {
        if (monsterLevel >= 6 && rand() % 100 < 30)
        {
            // 稀有掉落
            int idx = rand() % rareItems.size();
            loot.items.push_back(rareItems[idx]);
        }
        else
        {
            int idx = rand() % commonItems.size();
            loot.items.push_back(commonItems[idx]);
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
    ConsoleUI::printTitle("冒险结算");

    switch (status)
    {
    case AdventureStatus::SUCCESS:
    {
        // 全部转移
        std::cout << "结算结果: 成功通关！\n";
        std::cout << "获得金币: " << temporaryBackpack.gold << "\n";
        std::cout << "获得物品: " << temporaryBackpack.items.size() << " 件\n";

        player.gainGold(temporaryBackpack.gold);
        for (const auto& itemName : temporaryBackpack.items)
            addItemToInventory(itemName, inventory);
        break;
    }

    case AdventureStatus::ESCAPED:
    {
        // 随机丢失约30%物品
        int totalItems = static_cast<int>(temporaryBackpack.items.size());
        int loseCount = std::max(1, totalItems * 30 / 100); // 至少丢1件

        // 随机选择要丢失的物品
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

        // 保留未丢失的物品
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
        std::cout << "保留物品: " << keptCount << " 件\n";
        // 金币不保留（逃跑金币全丢）
        break;
    }

    case AdventureStatus::DEFEATED:
    {
        // 全部丢失
        std::cout << "结算结果: 战败\n";
        std::cout << "丢失金币: " << temporaryBackpack.gold << "\n";
        std::cout << "丢失物品: " << temporaryBackpack.items.size() << " 件\n";
        std::cout << "保留物品: 0 件\n";
        break;
    }

    default:
        break;
    }

    // 清空临时背包
    temporaryBackpack = {0, {}};
    ConsoleUI::pause();
}

// ============================================================
// 将物品名称转换为 Item 并加入背包
// ============================================================

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
