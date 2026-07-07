// Character.cpp
#include "Character.h"
#include "GameConfig.h"
#include "ConsoleUI.h"

#include <iostream>
#include <sstream>
#include <algorithm>

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

// 获得经验值，并在经验足够时自动升级。
void Character::gainExp(int amount)
{
    exp += amount;
    std::cout << "获得 " << amount << " 点经验值！\n";
    levelUpIfNeeded();
}

// 获得金币。
void Character::gainGold(int amount)
{
    gold += amount;
    std::cout << "获得 " << amount << " 金币！\n";
}

// 消耗金币。返回 true 表示扣除成功。
bool Character::costGold(int amount)
{
    if (gold >= amount)
    {
        gold -= amount;
        return true;
    }
    std::cout << "金币不足！需要 " << amount << " 金币，当前只有 " << gold << " 金币。\n";
    return false;
}

// 角色受到伤害，最低不低于 0。
void Character::takeDamage(int damage)
{
    hp -= damage;
    if (hp < 0)
    {
        hp = 0;
    }
}

// 恢复生命值，但不超过 maxHp。
void Character::heal(int amount)
{
    hp += amount;
    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

bool Character::isAlive() const
{
    return hp > 0;
}

// 检查经验是否满足升级条件，若满足则提升等级并增长属性。
// 使用 while 循环，因为经验可能一次性跨越多个等级。
void Character::levelUpIfNeeded()
{
    // 升级所需经验 = 当前等级 * 100
    while (exp >= level * 100)
    {
        exp -= level * 100;
        level += 1;
        maxHp += 20;
        attack += 5;
        defense += 2;
        hp = maxHp;  // 升级后回满生命

        std::cout << "★ 升级！你达到了等级 " << level << "！\n";
        std::cout << "  最大生命 +20 → " << maxHp << "\n";
        std::cout << "  攻击力   +5  → " << attack << "\n";
        std::cout << "  防御力   +2  → " << defense << "\n";
    }
}

// 增加攻击力。
void Character::addAttack(int value)
{
    attack += value;
}

// 增加防御力。
void Character::addDefense(int value)
{
    defense += value;
}

// 将角色数据序列化为字符串，用于保存到文件。
std::string Character::serialize() const
{
    std::ostringstream oss;
    oss << "name=" << name << "\n";
    oss << "level=" << level << "\n";
    oss << "hp=" << hp << "\n";
    oss << "maxHp=" << maxHp << "\n";
    oss << "exp=" << exp << "\n";
    oss << "gold=" << gold << "\n";
    oss << "attack=" << attack << "\n";
    oss << "defense=" << defense << "\n";
    return oss.str();
}

// 从字符串解析恢复角色数据。
void Character::deserialize(const std::string& data)
{
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line))
    {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "name")      name = value;
        else if (key == "level")    level = std::stoi(value);
        else if (key == "hp")       hp = std::stoi(value);
        else if (key == "maxHp")    maxHp = std::stoi(value);
        else if (key == "exp")      exp = std::stoi(value);
        else if (key == "gold")     gold = std::stoi(value);
        else if (key == "attack")   attack = std::stoi(value);
        else if (key == "defense")  defense = std::stoi(value);
    }
}

std::string Character::getName() const { return name; }
int Character::getLevel() const { return level; }
int Character::getHp() const { return hp; }
int Character::getMaxHp() const { return maxHp; }
int Character::getExp() const { return exp; }
int Character::getGold() const { return gold; }
int Character::getAttack() const { return attack; }
int Character::getDefense() const { return defense; }
