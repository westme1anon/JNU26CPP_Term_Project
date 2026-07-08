// BattleSystem.cpp
#include "BattleSystem.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "PathUtil.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
std::string makeHpBar(int hp, int maxHp, int width = 24)
{
    if (maxHp <= 0) return "[无效]";
    const int safeHp = std::max(0, std::min(hp, maxHp));
    const int filled = safeHp * width / maxHp;
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

std::vector<Enemy> defaultEnemies()
{
    return {
        Enemy("史莱姆", 45, 12, 3, 60, 30),
        Enemy("哥布林", 80, 18, 6, 110, 55),
        Enemy("巨龙", 200, 30, 15, 300, 200)
    };
}
}

BattleSystem::BattleSystem() { loadEnemies(); }

void BattleSystem::loadEnemies()
{
    enemies.clear();

    std::string resolved = PathUtil::resolvePath(GameConfig::ENEMIES_FILE_PATH);
    if (resolved.empty()) resolved = GameConfig::ENEMIES_FILE_PATH;

    std::ifstream input(resolved);
    if (input)
    {
        std::string line;
        while (std::getline(input, line))
        {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string name, hpText, attackText, defenseText, expText, goldText;
            if (std::getline(ss, name, ',') &&
                std::getline(ss, hpText, ',') &&
                std::getline(ss, attackText, ',') &&
                std::getline(ss, defenseText, ',') &&
                std::getline(ss, expText, ',') &&
                std::getline(ss, goldText, ','))
            {
                enemies.emplace_back(name,
                    std::stoi(hpText), std::stoi(attackText), std::stoi(defenseText),
                    std::stoi(expText), std::stoi(goldText));
            }
        }
    }

    if (enemies.empty())
        enemies = defaultEnemies();
}

void BattleSystem::showEnemyList() const
{
    ConsoleUI::printTitle("可挑战敌人");
    for (std::size_t i = 0; i < enemies.size(); ++i)
    {
        std::cout << (i + 1) << ". ";
        enemies[i].showInfo();
    }
}

Enemy BattleSystem::selectEnemy(int index) const
{
    if (index < 0 || index >= static_cast<int>(enemies.size()))
        return Enemy();
    return enemies[static_cast<std::size_t>(index)];
}

BattleResult BattleSystem::startBattle(Character& player)
{
    BattleResult result;
    result.playerWon = false;

    ConsoleUI::clearScreen();
    showEnemyList();

    const int choice = ConsoleUI::readInt("选择敌人(0 返回): ");
    if (choice == 0) return result;

    if (choice < 1 || choice > static_cast<int>(enemies.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "敌人选择无效。\n";
        ConsoleUI::resetColor();
        ConsoleUI::pause();
        return result;
    }

    Enemy enemy = selectEnemy(choice - 1);
    result.enemyName = enemy.getName();
    const int enemyMaxHp = enemy.getHp();
    const int playerStartHp = player.getHp();

    while (player.isAlive() && enemy.isAlive())
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("战斗");
        std::cout << player.getName() << " 生命值: " << player.getHp() << "/" << player.getMaxHp()
                  << " " << makeHpBar(player.getHp(), player.getMaxHp()) << "\n";
        std::cout << enemy.getName() << " 生命值: " << enemy.getHp() << "/" << enemyMaxHp
                  << " " << makeHpBar(enemy.getHp(), enemyMaxHp) << "\n\n";

        const int confirm = ConsoleUI::readInt("输入 1 发起攻击: ");
        if (confirm != 1) continue;

        const int playerDamage = calculateDamage(player.getAttack(), enemy.getDefense());
        enemy.takeDamage(playerDamage);
        std::cout << player.getName() << " 对 " << enemy.getName()
                  << " 造成了 " << playerDamage << " 点伤害。\n";

        if (!enemy.isAlive()) break;

        const int enemyDamage = calculateDamage(enemy.getAttack(), player.getDefense());
        player.takeDamage(enemyDamage);
        std::cout << enemy.getName() << " 对 " << player.getName()
                  << " 造成了 " << enemyDamage << " 点伤害。\n";

        ConsoleUI::pause();
    }

    if (player.isAlive())
    {
        result.playerWon = true;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "战斗胜利！你击败了 " << enemy.getName() << "。\n";
        ConsoleUI::resetColor();
        player.gainExp(enemy.getRewardExp());
        player.gainGold(enemy.getRewardGold());
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "战斗失败。" << player.getName() << " 被 " << enemy.getName() << " 击倒了。\n";
        ConsoleUI::resetColor();
        const int fallbackHp = std::max(1, player.getMaxHp() / 4);
        player.heal(fallbackHp);
        if (playerStartHp > 0 && player.getHp() == fallbackHp)
            std::cout << "战斗后恢复了 " << fallbackHp << " 点生命值，冒险可以继续。\n";
    }

    ConsoleUI::pause();
    return result;
}

int BattleSystem::calculateDamage(int attack, int defense) const
{
    return std::max(1, attack - defense);
}
