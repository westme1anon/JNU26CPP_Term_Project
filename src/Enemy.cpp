// Enemy.cpp
#include "Enemy.h"

#include <algorithm>
#include <iostream>

Enemy::Enemy()
    : name("\xe6\x9c\xaa\xe7\x9f\xa5\xe6\x95\x8c\xe4\xba\xba"),
      hp(50),
      attack(10),
      defense(3),
      spd(5),
      tier(EnemyTier::Minion),
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
      spd(5),
      tier(EnemyTier::Minion),
      rewardExp(rewardExp),
      rewardGold(rewardGold)
{
}

Enemy::Enemy(
    const std::string& name,
    int hp,
    int attack,
    int defense,
    int spd,
    EnemyTier tier,
    int rewardExp,
    int rewardGold,
    const std::vector<std::string>& lootItems
)
    : name(name),
      hp(hp),
      attack(attack),
      defense(defense),
      spd(spd),
      tier(tier),
      rewardExp(rewardExp),
      rewardGold(rewardGold),
      lootItems(lootItems)
{
}

void Enemy::showInfo() const
{
    const char* tierName = "Minion";
    if (tier == EnemyTier::Elite) tierName = "Elite";
    else if (tier == EnemyTier::Boss) tierName = "Boss";

    std::cout << "[" << tierName << "] " << name
              << " | HP: " << hp
              << " | ATK: " << attack
              << " | DEF: " << defense
              << " | SPD: " << spd
              << " | EXP: " << rewardExp
              << " | GOLD: " << rewardGold
              << " | Loot: " << lootItems.size() << " items\n";
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

void Enemy::applyTierMultiplier()
{
    double mult = 1.0;
    switch (tier)
    {
    case EnemyTier::Minion: mult = 1.0; break;
    case EnemyTier::Elite:  mult = 1.5; break;
    case EnemyTier::Boss:   mult = 3.0; break;
    }
    hp       = static_cast<int>(hp * mult);
    attack   = static_cast<int>(attack * mult);
    defense  = static_cast<int>(defense * mult);
    spd      = static_cast<int>(spd * mult);
    rewardExp  = static_cast<int>(rewardExp * mult);
    rewardGold = static_cast<int>(rewardGold * mult);
}

void Enemy::addLootItem(const std::string& item)
{
    lootItems.push_back(item);
}

std::string Enemy::getName() const { return name; }
int Enemy::getHp() const { return hp; }
int Enemy::getAttack() const { return attack; }
int Enemy::getDefense() const { return defense; }
int Enemy::getSpd() const { return spd; }
EnemyTier Enemy::getTier() const { return tier; }
int Enemy::getRewardExp() const { return rewardExp; }
int Enemy::getRewardGold() const { return rewardGold; }
const std::vector<std::string>& Enemy::getLootItems() const { return lootItems; }
