// BattleSystem.h
#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "Character.h"
#include "Enemy.h"
#include "EnemyFactory.h"
#include "AdventureManager.h"

#include <string>
#include <vector>

// ============================================================
// BattleResult
// ------------------------------------------------------------
// 战斗结果信息，用于驱动任务系统事件。
// ============================================================

struct BattleResult
{
    BattleOutcome outcome = BattleOutcome::PLAYER_WIN;
    std::string enemyName;
};

// ============================================================
// BattleSystem
// ------------------------------------------------------------
// 战斗系统类。
// 负责敌人加载、敌人选择、回合制战斗、奖励结算等。
// ============================================================

class BattleSystem
{
private:
    EnemyFactory enemyFactory;

public:
    BattleSystem();

    // 加载敌人数据（从 JSON）。
    void loadEnemies();

    // 获取敌人工厂引用。
    EnemyFactory& getEnemyFactory() { return enemyFactory; }

    // 显示敌人模板列表。
    void showEnemyList() const;

    // 根据下标选择敌人模板。
    Enemy selectEnemy(int index) const;

    // 获取敌人模板总数。
    int enemyCount() const;

    // 启动一场自由战斗（玩家选择敌人），返回战斗结果。
    BattleResult startBattle(Character& player);

    // 启动一场冒险战斗（根据阶段生成敌人），返回战斗结果。
    BattleResult startBattle(Character& player, int stage, bool isBoss);

    // 计算伤害值。
    // 基础公式：max(1, attack - defense)。
    int calculateDamage(int attack, int defense) const;

private:
    // 单敌人战斗循环。
    bool fightOneEnemy(Character& player, const Enemy& enemy) const;
};

#endif
