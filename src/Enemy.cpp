// Enemy.cpp
#include "Enemy.h"

#include <algorithm>
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
    std::cout << name
              << " | 生命值: " << hp
              << " | 攻击: " << attack
              << " | 防御: " << defense
              << " | 奖励: " << rewardExp << " 经验, " << rewardGold << " 金币\n";
}

void Enemy::takeDamage(int damage)
{
    if (damage <= 0)
    {
        return;
    }

    hp = std::max(0, hp - damage);
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
