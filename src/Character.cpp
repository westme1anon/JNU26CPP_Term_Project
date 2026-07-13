// Character.cpp
#include "Character.h"
#include "GameConfig.h"
#include "ConsoleUI.h"

#include <iostream>
#include <sstream>
#include <algorithm>

Character::Character()
    : name("\346\234\252\345\221\275\345\220\215"),
      level(GameConfig::DEFAULT_LEVEL),
      hp(GameConfig::DEFAULT_HP),
      maxHp(GameConfig::DEFAULT_HP),
      exp(GameConfig::DEFAULT_EXP),
      gold(GameConfig::DEFAULT_GOLD),
      attack(GameConfig::DEFAULT_ATTACK),
      defense(GameConfig::DEFAULT_DEFENSE),
      invincible(false)
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
    invincible = false;
}

void Character::showInfo() const
{
    ConsoleUI::printTitle("\350\247\222\350\211\262\344\277\241\346\201\257");

    std::cout << "\345\247\223\345\220\215: " << name << "\n";
    std::cout << "\347\255\211\347\272\247: " << level << "\n";
    std::cout << "\347\224\237\345\221\275\345\200\274: " << hp << "/" << maxHp << "\n";
    std::cout << "\347\273\217\351\252\214\345\200\274: " << exp << "\n";
    std::cout << "\351\207\221\345\270\201: " << gold << "\n";
    std::cout << "\346\224\273\345\207\273\345\212\233: " << attack << "\n";
    std::cout << "\351\230\262\345\276\241\345\212\233: " << defense << "\n";
}

void Character::gainExp(int amount)
{
    exp += amount;
    std::cout << "\350\216\267\345\276\227 " << amount << " \347\202\271\347\273\217\351\252\214\345\200\274\357\274\201\n";
    levelUpIfNeeded();
}

void Character::gainGold(int amount)
{
    gold += amount;
    std::cout << "\350\216\267\345\276\227 " << amount << " \351\207\221\345\270\201\357\274\201\n";
}

bool Character::costGold(int amount)
{
    if (gold >= amount)
    {
        gold -= amount;
        return true;
    }
    std::cout << "\351\207\221\345\270\201\344\270\215\350\266\263\357\274\201\351\234\200\350\246\201 " << amount << " \351\207\221\345\270\201\357\274\214\345\275\223\345\211\215\345\217\252\346\234\211 " << gold << " \351\207\221\345\270\201\343\200\202\n";
    return false;
}

void Character::takeDamage(int damage)
{
    if (invincible)
    {
        std::cout << "\343\200\220\346\227\240\346\225\214\346\250\241\345\274\217\343\200\221\345\205\215\347\226\253\344\272\206 " << damage << " \347\202\271\344\274\244\345\256\263\357\274\201\n";
        return;
    }
    hp -= damage;
    if (hp < 0)
    {
        hp = 0;
    }
}

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

void Character::setLevel(int newLevel)
{
    if (newLevel < 1) newLevel = 1;
    level = newLevel;
    maxHp = GameConfig::DEFAULT_HP + (level - 1) * 20;
    attack = GameConfig::DEFAULT_ATTACK + (level - 1) * 5;
    defense = GameConfig::DEFAULT_DEFENSE + (level - 1) * 2;
    hp = maxHp;
}

void Character::setMaxHp(int newMaxHp)
{
    if (newMaxHp < 1) newMaxHp = 1;
    maxHp = newMaxHp;
    if (hp > maxHp) hp = maxHp;
}

void Character::maxStats()
{
    level = 99;
    maxHp = 9999;
    hp = 9999;
    attack = 999;
    defense = 999;
    exp = 0;
}

bool Character::isInvincible() const
{
    return invincible;
}

void Character::setInvincible(bool value)
{
    invincible = value;
}

void Character::levelUpIfNeeded()
{
    while (exp >= level * 100)
    {
        exp -= level * 100;
        level += 1;
        maxHp += 20;
        attack += 5;
        defense += 2;
        hp = maxHp;

        std::cout << "\342\230\205 \345\215\207\347\272\247\357\274\201\344\275\240\350\276\276\345\210\260\344\272\206\347\255\211\347\272\247 " << level << "\357\274\201\n";
        std::cout << "  \346\234\200\345\244\247\347\224\237\345\221\275 +20 \342\206\222 " << maxHp << "\n";
        std::cout << "  \346\224\273\345\207\273\345\212\233   +5  \342\206\222 " << attack << "\n";
        std::cout << "  \351\230\262\345\276\241\345\212\233   +2  \342\206\222 " << defense << "\n";
    }
}

void Character::addAttack(int value)
{
    attack += value;
}

void Character::addDefense(int value)
{
    defense += value;
}

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
    oss << "invincible=" << (invincible ? 1 : 0) << "\n";
    return oss.str();
}

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
        else if (key == "invincible") invincible = (std::stoi(value) != 0);
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
