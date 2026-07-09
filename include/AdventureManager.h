#ifndef ADVENTURE_MANAGER_H
#define ADVENTURE_MANAGER_H

#include "BattleSystem.h"
#include "BattleTypes.h"
#include "Character.h"
#include "Enemy.h"
#include "EnemyFactory.h"
#include "Inventory.h"

#include <string>
#include <vector>

enum class AdventureStatus
{
    IN_PROGRESS,
    SUCCESS,
    ESCAPED,
    DEFEATED
};

struct Loot
{
    int gold = 0;
    std::vector<std::string> items;
};

class AdventureManager
{
private:
    int currentStage;
    int maxStages;
    Loot temporaryBackpack;
    EnemyFactory enemyFactory;
    BattleSystem battleSystem;

public:
    AdventureManager();

    void loadEnemyData();
    AdventureStatus startAdventure(Character& player, Inventory& inventory);

    const Loot& getTemporaryBackpack() const { return temporaryBackpack; }
    int getCurrentStage() const { return currentStage; }
    int getMaxStages() const { return maxStages; }

private:
    BattleOutcome triggerBattle(Character& player, const std::vector<Enemy>& enemies);
    Loot generateLoot(const std::vector<Enemy>& enemies);
    void settleAdventure(AdventureStatus status, Character& player, Inventory& inventory);
    void addItemToInventory(const std::string& itemName, Inventory& inventory);
};

#endif
