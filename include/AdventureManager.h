// AdventureManager.h
#ifndef ADVENTURE_MANAGER_H
#define ADVENTURE_MANAGER_H

#include "Character.h"
#include "Inventory.h"

#include <string>
#include <vector>

// ============================================================
// AdventureStatus
// ------------------------------------------------------------
// 冒险状态枚举。
// ============================================================

enum class AdventureStatus
{
    IN_PROGRESS,  // 冒险进行中
    SUCCESS,      // 成功通关
    ESCAPED,      // 中途逃跑
    DEFEATED      // 战败（死亡）
};

// ============================================================
// BattleOutcome
// ------------------------------------------------------------
// 单场战斗结果枚举。
// ============================================================

enum class BattleOutcome
{
    PLAYER_WIN,   // 玩家胜利
    PLAYER_ESCAPE,// 玩家逃跑
    PLAYER_DEAD   // 玩家战死
};

// ============================================================
// Loot
// ------------------------------------------------------------
// 战利品结构体，用于冒险途中的临时背包。
// ============================================================

struct Loot
{
    int gold = 0;
    std::vector<std::string> items;
};

// ============================================================
// AdventureManager
// ------------------------------------------------------------
// 冒险管理器。
// 负责冒险核心循环：进入冒险 → 连续战斗（难度递增）→ 击败Boss或逃跑 → 带出战利品。
// 逃跑扣除约30%临时战利品，战败则全部丢失。
// ============================================================

class AdventureManager
{
private:
    int currentStage;
    int maxStages;
    Loot temporaryBackpack;

public:
    AdventureManager();

    // 启动一次完整的冒险流程。
    // 返回最终的冒险状态。
    AdventureStatus startAdventure(Character& player, Inventory& inventory);

    // 获取当前临时背包中的战利品（调试用）。
    const Loot& getTemporaryBackpack() const { return temporaryBackpack; }
    int getCurrentStage() const { return currentStage; }
    int getMaxStages() const { return maxStages; }

private:
    // 占位战斗接口。
    // 当前使用控制台输入（1-胜利/2-逃跑/3-战败）模拟战斗结果。
    BattleOutcome triggerBattle(int monsterLevel, bool isBoss);

    // 根据怪物等级生成战利品。
    Loot generateLoot(int monsterLevel);

    // 结算冒险，根据状态将临时战利品转移到玩家永久背包。
    void settleAdventure(AdventureStatus status, Character& player, Inventory& inventory);

    // 根据物品名称创建对应的 Item 对象并加入背包。
    void addItemToInventory(const std::string& itemName, Inventory& inventory);
};

#endif
