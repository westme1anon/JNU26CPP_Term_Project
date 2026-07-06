// BattleSystem.h
#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "Character.h"
#include "Enemy.h"

#include <vector>

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
    // 后续可从 data/enemies.txt 读取。
    void loadEnemies();

    // 显示敌人列表。
    void showEnemyList() const;

    // 根据下标选择敌人。
    Enemy selectEnemy(int index) const;

    // 启动一场完整战斗。
    void startBattle(Character& player);

    // 计算伤害值。
    // 基础公式：max(1, attack - defense)。
    int calculateDamage(int attack, int defense) const;
};

#endif