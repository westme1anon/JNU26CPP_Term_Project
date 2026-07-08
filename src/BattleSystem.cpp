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
    if (maxHp <= 0) return "[" + std::string("\xe6\x97\xa0\xe6\x95\x88") + "]";
    const int safeHp = std::max(0, std::min(hp, maxHp));
    const int filled = safeHp * width / maxHp;
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

std::vector<Enemy> defaultEnemies()
{
    return {
        Enemy("\xe5\x8f\xb2\xe8\x8e\xb1\xe5\xa7\x86", 45, 12, 3, 60, 30),
        Enemy("\xe5\x93\xa5\xe5\xb8\x83\xe6\x9e\x97", 80, 18, 6, 110, 55),
        Enemy("\xe5\xb7\xa8\xe9\xbe\x99", 200, 30, 15, 300, 200)
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
    ConsoleUI::printTitle("\xe5\x8f\xaf\xe6\x8c\x91\xe6\x88\x98\xe6\x95\x8c\xe4\xba\xba");
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

int BattleSystem::enemyCount() const
{
    return static_cast<int>(enemies.size());
}

Enemy BattleSystem::selectEnemyByLevel(int monsterLevel) const
{
    if (enemies.empty())
        return Enemy("\xe6\x9c\xaa\xe7\x9f\xa5\xe6\x80\xaa\xe7\x89\xa9", monsterLevel * 15, monsterLevel * 3, monsterLevel, monsterLevel * 20, monsterLevel * 10);

    int bestIdx = 0;
    int bestDiff = std::abs(monsterLevel - 2);

    for (std::size_t i = 0; i < enemies.size(); ++i)
    {
        int estimatedLevel = enemies[i].getHp() / 15;
        int diff = std::abs(monsterLevel - estimatedLevel);
        if (diff < bestDiff)
        {
            bestDiff = diff;
            bestIdx = static_cast<int>(i);
        }
    }
    return enemies[bestIdx];
}

BattleResult BattleSystem::startBattle(Character& player)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    ConsoleUI::clearScreen();
    showEnemyList();

    const int choice = ConsoleUI::readInt("\xe9\x80\x89\xe6\x8b\xa9\xe6\x95\x8c\xe4\xba\xba(0 \xe8\xbf\x94\xe5\x9b\x9e): ");
    if (choice == 0)
    {
        result.outcome = BattleOutcome::PLAYER_ESCAPE;
        return result;
    }

    if (choice < 1 || choice > static_cast<int>(enemies.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "\xe6\x95\x8c\xe4\xba\xba\xe9\x80\x89\xe6\x8b\xa9\xe6\x97\xa0\xe6\x95\x88\xe3\x80\x82\n";
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
        ConsoleUI::printTitle("\xe6\x88\x98\xe6\x96\x97");
        std::cout << player.getName() << " \xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc: " << player.getHp() << "/" << player.getMaxHp()
                  << " " << makeHpBar(player.getHp(), player.getMaxHp()) << "\n";
        std::cout << enemy.getName() << " \xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc: " << enemy.getHp() << "/" << enemyMaxHp
                  << " " << makeHpBar(enemy.getHp(), enemyMaxHp) << "\n\n";

        const int confirm = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5 1 \xe5\x8f\x91\xe8\xb5\xb7\xe6\x94\xbb\xe5\x87\xbb: ");
        if (confirm != 1) continue;

        const int playerDamage = calculateDamage(player.getAttack(), enemy.getDefense());
        enemy.takeDamage(playerDamage);
        std::cout << player.getName() << " \xe5\xaf\xb9 " << enemy.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << playerDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        if (!enemy.isAlive()) break;

        const int enemyDamage = calculateDamage(enemy.getAttack(), player.getDefense());
        player.takeDamage(enemyDamage);
        std::cout << enemy.getName() << " \xe5\xaf\xb9 " << player.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << enemyDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        ConsoleUI::pause();
    }

    if (player.isAlive())
    {
        result.outcome = BattleOutcome::PLAYER_WIN;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "\xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\xef\xbc\x81\xe4\xbd\xa0\xe5\x87\xbb\xe8\xb4\xa5\xe4\xba\x86 " << enemy.getName() << "\xe3\x80\x82\n";
        ConsoleUI::resetColor();
        player.gainExp(enemy.getRewardExp());
        player.gainGold(enemy.getRewardGold());
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "\xe6\x88\x98\xe6\x96\x97\xe5\xa4\xb1\xe8\xb4\xa5\xe3\x80\x82" << player.getName() << " \xe8\xa2\xab " << enemy.getName() << " \xe5\x87\xbb\xe5\x80\x92\xe4\xba\x86\xe3\x80\x82\n";
        ConsoleUI::resetColor();
        const int fallbackHp = std::max(1, player.getMaxHp() / 4);
        player.heal(fallbackHp);
        if (playerStartHp > 0 && player.getHp() == fallbackHp)
            std::cout << "\xe6\x88\x98\xe6\x96\x97\xe5\x90\x8e\xe6\x81\xa2\xe5\xa4\x8d\xe4\xba\x86 " << fallbackHp << " \xe7\x82\xb9\xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc\xef\xbc\x8c\xe5\x86\x92\xe9\x99\xa9\xe5\x8f\xaf\xe4\xbb\xa5\xe7\xbb\xa7\xe7\xbb\xad\xe3\x80\x82\n";
    }

    ConsoleUI::pause();
    return result;
}

BattleResult BattleSystem::startBattle(Character& player, int monsterLevel, bool isBoss)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    Enemy enemy = selectEnemyByLevel(monsterLevel);

    if (isBoss)
    {
        enemy = Enemy("Boss." + enemy.getName(),
            enemy.getHp() * 2,
            enemy.getAttack() + 5,
            enemy.getDefense() + 3,
            enemy.getRewardExp() * 3,
            enemy.getRewardGold() * 3);
    }

    result.enemyName = enemy.getName();
    const int enemyMaxHp = enemy.getHp();
    const int playerStartHp = player.getHp();

    ConsoleUI::printLine('-');
    ConsoleUI::setColor(isBoss ? GameConfig::COLOR_WARNING : GameConfig::COLOR_TITLE);
    std::cout << "\xe9\x81\xad\xe9\x81\x87 " << enemy.getName() << "\xef\xbc\x81\xef\xbc\x88\xe7\xad\x89\xe7\xba\xa7\xe7\xba\xa6 " << monsterLevel << "\xef\xbc\x89\n";
    ConsoleUI::resetColor();

    while (player.isAlive() && enemy.isAlive())
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("\xe6\x88\x98\xe6\x96\x97");
        std::cout << player.getName() << " \xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc: " << player.getHp() << "/" << player.getMaxHp()
                  << " " << makeHpBar(player.getHp(), player.getMaxHp()) << "\n";
        std::cout << enemy.getName() << " \xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc: " << enemy.getHp() << "/" << enemyMaxHp
                  << " " << makeHpBar(enemy.getHp(), enemyMaxHp) << "\n\n";

        const int confirm = ConsoleUI::readInt("\xe8\xbe\x93\xe5\x85\xa5 1 \xe5\x8f\x91\xe8\xb5\xb7\xe6\x94\xbb\xe5\x87\xbb: ");
        if (confirm != 1) continue;

        const int playerDamage = calculateDamage(player.getAttack(), enemy.getDefense());
        enemy.takeDamage(playerDamage);
        std::cout << player.getName() << " \xe5\xaf\xb9 " << enemy.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << playerDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        if (!enemy.isAlive()) break;

        const int enemyDamage = calculateDamage(enemy.getAttack(), player.getDefense());
        player.takeDamage(enemyDamage);
        std::cout << enemy.getName() << " \xe5\xaf\xb9 " << player.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << enemyDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        ConsoleUI::pause();
    }

    if (player.isAlive())
    {
        result.outcome = BattleOutcome::PLAYER_WIN;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "\xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\xef\xbc\x81\xe4\xbd\xa0\xe5\x87\xbb\xe8\xb4\xa5\xe4\xba\x86 " << enemy.getName() << "\xe3\x80\x82\n";
        ConsoleUI::resetColor();
        player.gainExp(enemy.getRewardExp());
        player.gainGold(enemy.getRewardGold());
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "\xe6\x88\x98\xe6\x96\x97\xe5\xa4\xb1\xe8\xb4\xa5\xe3\x80\x82" << player.getName() << " \xe8\xa2\xab " << enemy.getName() << " \xe5\x87\xbb\xe5\x80\x92\xe4\xba\x86\xe3\x80\x82\n";
        ConsoleUI::resetColor();
        const int fallbackHp = std::max(1, player.getMaxHp() / 4);
        player.heal(fallbackHp);
        if (playerStartHp > 0 && player.getHp() == fallbackHp)
            std::cout << "\xe6\x88\x98\xe6\x96\x97\xe5\x90\x8e\xe6\x81\xa2\xe5\xa4\x8d\xe4\xba\x86 " << fallbackHp << " \xe7\x82\xb9\xe7\x94\x9f\xe5\x91\xbd\xe5\x80\xbc\xef\xbc\x8c\xe5\x86\x92\xe9\x99\xa9\xe5\x8f\xaf\xe4\xbb\xa5\xe7\xbb\xa7\xe7\xbb\xad\xe3\x80\x82\n";
    }

    ConsoleUI::pause();
    return result;
}

int BattleSystem::calculateDamage(int attack, int defense) const
{
    return std::max(1, attack - defense);
}
