// BattleSystem.h
#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "Character.h"
#include "Enemy.h"
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
// 负责敌人加载、选择敌人、回合制战斗、奖励结算等。
// ============================================================

class BattleSystem
{
private:
    std::vector<Enemy> enemies;

public:
    BattleSystem();

    // 加载敌人数据。
    void loadEnemies();

    // 显示敌人列表。
    void showEnemyList() const;

    // 根据下标选择敌人。
    Enemy selectEnemy(int index) const;

    // 获取敌人总数。
    int enemyCount() const;

    // 根据怪物等级选取合适的敌人。
    Enemy selectEnemyByLevel(int monsterLevel) const;

    // 启动一场完整战斗，返回战斗结果。
    BattleResult startBattle(Character& player);

    // 根据怪物等级和是否为Boss启动一场战斗。
    BattleResult startBattle(Character& player, int monsterLevel, bool isBoss);

    // 计算伤害值。
    // 基础公式：max(1, attack - defense)。
    int calculateDamage(int attack, int defense) const;
};

#endif
