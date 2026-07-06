// Enemy.h
#ifndef ENEMY_H
#define ENEMY_H

#include <string>

// ============================================================
// Enemy
// ------------------------------------------------------------
// 敌人类。
// 表示战斗中的怪物或对手，包含生命、攻击、防御、奖励等信息。
// ============================================================

class Enemy
{
private:
    std::string name;
    int hp;
    int attack;
    int defense;
    int rewardExp;
    int rewardGold;

public:
    Enemy();
    Enemy(
        const std::string& name,
        int hp,
        int attack,
        int defense,
        int rewardExp,
        int rewardGold
    );

    // 显示敌人信息。
    void showInfo() const;

    // 敌人受到伤害。
    void takeDamage(int damage);

    // 判断敌人是否存活。
    bool isAlive() const;

    std::string getName() const;
    int getHp() const;
    int getAttack() const;
    int getDefense() const;
    int getRewardExp() const;
    int getRewardGold() const;
};

#endif