// BattleSystem.cpp
#include "BattleSystem.h"
#include "ConsoleUI.h"

#include <iostream>
#include <algorithm>

BattleSystem::BattleSystem()
{
    loadEnemies();
}

void BattleSystem::loadEnemies()
{
    // TODO: 从文件加载敌人数据。
    // 临时可初始化三种敌人：
    // 1. 作业怪
    // 2. 考试魔王
    // 3. 社团挑战者
}

void BattleSystem::showEnemyList() const
{
    // TODO: 遍历 enemies，显示可挑战敌人。
}

Enemy BattleSystem::selectEnemy(int index) const
{
    // TODO: 校验 index，返回对应敌人副本。
    return Enemy();
}

void BattleSystem::startBattle(Character& player)
{
    // TODO:
    // 1. 显示敌人列表
    // 2. 玩家选择敌人
    // 3. 回合制攻击
    // 4. 判断胜负
    // 5. 发放经验和金币奖励
}

int BattleSystem::calculateDamage(int attack, int defense) const
{
    // TODO: 使用 std::max 保证最低伤害为 1。
    return std::max(1, attack - defense);
}