// Enemy.cpp
#include "Enemy.h"

#include <iostream>

Enemy::Enemy()
    : name("未知敌人"),
      hp(50),
      attack(10),
      defense(3),
      rewardExp(10),
      rewardGold(10)
{
}

Enemy::Enemy(
    const std::string& name,
    int hp,
    int attack,
    int defense,
    int rewardExp,
    int rewardGold
)
    : name(name),
      hp(hp),
      attack(attack),
      defense(defense),
      rewardExp(rewardExp),
      rewardGold(rewardGold)
{
}

void Enemy::showInfo() const
{
    // TODO: 输出敌人名称、生命值、攻击、防御、奖励。
}

void Enemy::takeDamage(int damage)
{
    // TODO: 扣除敌人生命值，最低不低于 0。
}

bool Enemy::isAlive() const
{
    return hp > 0;
}

std::string Enemy::getName() const { return name; }
int Enemy::getHp() const { return hp; }
int Enemy::getAttack() const { return attack; }
int Enemy::getDefense() const { return defense; }
int Enemy::getRewardExp() const { return rewardExp; }
int Enemy::getRewardGold() const { return rewardGold; }