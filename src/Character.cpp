// Character.cpp
#include "Character.h"
#include "GameConfig.h"
#include "ConsoleUI.h"

#include <iostream>
#include <sstream>

Character::Character()
    : name("未命名"),
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
    // TODO: 增加经验并调用 levelUpIfNeeded。
}

void Character::gainGold(int amount)
{
    // TODO: 增加金币。
}

bool Character::costGold(int amount)
{
    // TODO: 判断金币是否足够，足够则扣除。
    return false;
}

void Character::takeDamage(int damage)
{
    // TODO: 根据伤害值扣除生命。
}

void Character::heal(int amount)
{
    // TODO: 恢复生命值，不能超过 maxHp。
}

bool Character::isAlive() const
{
    return hp > 0;
}

void Character::levelUpIfNeeded()
{
    // TODO: 根据经验值判断是否升级，并提升 maxHp、attack、defense 等属性。
}

void Character::addAttack(int value)
{
    // TODO: 增加攻击力。
}

void Character::addDefense(int value)
{
    // TODO: 增加防御力。
}

std::string Character::serialize() const
{
    // TODO: 将角色属性拼接为可保存字符串。
    return "";
}

void Character::deserialize(const std::string& data)
{
    // TODO: 从字符串解析角色属性。
}

std::string Character::getName() const { return name; }
int Character::getLevel() const { return level; }
int Character::getHp() const { return hp; }
int Character::getMaxHp() const { return maxHp; }
int Character::getExp() const { return exp; }
int Character::getGold() const { return gold; }
int Character::getAttack() const { return attack; }
int Character::getDefense() const { return defense; }