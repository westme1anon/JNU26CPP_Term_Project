// Character.cpp
#include "Character.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <algorithm>
#include <iostream>
#include <sstream>

Character::Character()
    : name("未命名角色"),
      level(GameConfig::DEFAULT_LEVEL),
      hp(GameConfig::DEFAULT_HP),
      maxHp(GameConfig::DEFAULT_HP),
      exp(GameConfig::DEFAULT_EXP),
      gold(GameConfig::DEFAULT_GOLD),
      attack(GameConfig::DEFAULT_ATTACK),
      defense(GameConfig::DEFAULT_DEFENSE)
{
}

void Character::create(const std::string& characterName)
{
    name = characterName;
    level = GameConfig::DEFAULT_LEVEL;
    hp = GameConfig::DEFAULT_HP;
    maxHp = GameConfig::DEFAULT_HP;
    exp = GameConfig::DEFAULT_EXP;
    gold = GameConfig::DEFAULT_GOLD;
    attack = GameConfig::DEFAULT_ATTACK;
    defense = GameConfig::DEFAULT_DEFENSE;
}

void Character::showInfo() const
{
    ConsoleUI::printTitle("角色信息");
    std::cout << "姓名: " << name << "\n";
    std::cout << "等级: " << level << "\n";
    std::cout << "生命值: " << hp << "/" << maxHp << "\n";
    std::cout << "经验值: " << exp << "\n";
    std::cout << "金币: " << gold << "\n";
    std::cout << "攻击力: " << attack << "\n";
    std::cout << "防御力: " << defense << "\n";
}

void Character::gainExp(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    exp += amount;
    std::cout << name << " 获得了 " << amount << " 点经验值。\n";
    levelUpIfNeeded();
}

void Character::gainGold(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    gold += amount;
    std::cout << name << " 获得了 " << amount << " 枚金币。\n";
}

bool Character::costGold(int amount)
{
    if (amount < 0 || gold < amount)
    {
        return false;
    }

    gold -= amount;
    return true;
}

void Character::takeDamage(int damage)
{
    if (damage <= 0)
    {
        return;
    }

    hp = std::max(0, hp - damage);
}

void Character::heal(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    hp = std::min(maxHp, hp + amount);
}

bool Character::isAlive() const
{
    return hp > 0;
}

void Character::levelUpIfNeeded()
{
    while (exp >= level * 100)
    {
        exp -= level * 100;
        ++level;
        maxHp += 20;
        attack += 5;
        defense += 2;
        hp = maxHp;

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "升级！" << name << " 达到了 " << level << " 级。\n";
        std::cout << "最大生命值 +20，攻击力 +5，防御力 +2，生命值已回满。\n";
        ConsoleUI::resetColor();
    }
}

void Character::addAttack(int value)
{
    if (value > 0)
    {
        attack += value;
    }
}

void Character::addDefense(int value)
{
    if (value > 0)
    {
        defense += value;
    }
}

std::string Character::serialize() const
{
    std::ostringstream oss;
    oss << name << '|'
        << level << '|'
        << hp << '|'
        << maxHp << '|'
        << exp << '|'
        << gold << '|'
        << attack << '|'
        << defense;
    return oss.str();
}

void Character::deserialize(const std::string& data)
{
    std::stringstream ss(data);
    std::string token;

    if (!std::getline(ss, name, '|')) return;
    if (!std::getline(ss, token, '|')) return;
    level = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    hp = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    maxHp = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    exp = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    gold = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    attack = std::stoi(token);
    if (!std::getline(ss, token, '|')) return;
    defense = std::stoi(token);
}

std::string Character::getName() const { return name; }
int Character::getLevel() const { return level; }
int Character::getHp() const { return hp; }
int Character::getMaxHp() const { return maxHp; }
int Character::getExp() const { return exp; }
int Character::getGold() const { return gold; }
int Character::getAttack() const { return attack; }
int Character::getDefense() const { return defense; }
