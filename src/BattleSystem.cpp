// BattleSystem.cpp
#include "BattleSystem.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "PathUtil.h"

#include <algorithm>
#include <iostream>
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
}

BattleSystem::BattleSystem() { loadEnemies(); }

void BattleSystem::loadEnemies()
{
    std::string resolved = PathUtil::resolvePath(GameConfig::ENEMIES_FILE_PATH);
    if (resolved.empty()) resolved = GameConfig::ENEMIES_FILE_PATH;
    enemyFactory.loadFromJson(resolved);
}

void BattleSystem::showEnemyList() const
{
    ConsoleUI::printTitle("\xe5\x8f\xaf\xe6\x8c\x91\xe6\x88\x98\xe6\x95\x8c\xe4\xba\xba");

    int count = enemyFactory.templateCount();
    if (count == 0)
    {
        std::cout << "\xe6\xb2\xa1\xe6\x9c\x89\xe5\x8f\xaf\xe7\x94\xa8\xe7\x9a\x84\xe6\x95\x8c\xe4\xba\xba\xe6\xa8\xa1\xe6\x9d\xbf\xe3\x80\x82\n";
        return;
    }

    // Show first enemy from factory as representative
    const Enemy& sample = enemyFactory.pickRandomTemplate();
    std::cout << "\xe5\x85\xb1 " << count << " \xe7\xa7\x8d\xe6\x95\x8c\xe4\xba\xba\xe6\xa8\xa1\xe6\x9d\xbf\n";
    std::cout << "\xe7\xa4\xba\xe4\xbe\x8b: ";
    sample.showInfo();
}

Enemy BattleSystem::selectEnemy(int index) const
{
    // Use index to get a specific template
    int count = enemyFactory.templateCount();
    if (count == 0 || index < 0 || index >= count)
    {
        // Fallback to random
        const Enemy& tmpl = enemyFactory.pickRandomTemplate();
        Enemy e(tmpl.getName(), tmpl.getHp(), tmpl.getAttack(), tmpl.getDefense(),
                tmpl.getSpd(), EnemyTier::Minion,
                tmpl.getRewardExp(), tmpl.getRewardGold(), tmpl.getLootItems());
        e.applyTierMultiplier();
        return e;
    }

    const Enemy& tmpl = enemyFactory.getTemplate(index);
    Enemy e(
        tmpl.getName(),
        tmpl.getHp(),
        tmpl.getAttack(),
        tmpl.getDefense(),
        tmpl.getSpd(),
        EnemyTier::Minion,
        tmpl.getRewardExp(),
        tmpl.getRewardGold(),
        tmpl.getLootItems()
    );
    e.applyTierMultiplier();
    return e;
}

int BattleSystem::enemyCount() const
{
    return enemyFactory.templateCount();
}

// ============================================================
// 自由战斗（选择敌人战斗）
// ============================================================

BattleResult BattleSystem::startBattle(Character& player)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    ConsoleUI::clearScreen();
    ConsoleUI::printTitle("\xe8\x87\xaa\xe7\x94\xb1\xe6\x88\x98\xe6\x96\x97");

    int count = enemyFactory.templateCount();
    if (count == 0)
    {
        std::cout << "\xe6\xb2\xa1\xe6\x9c\x89\xe5\x8f\xaf\xe7\x94\xa8\xe7\x9a\x84\xe6\x95\x8c\xe4\xba\xba\xe3\x80\x82\n";
        ConsoleUI::pause();
        return result;
    }

    // Generate a random enemy
    Enemy enemy = selectEnemy(0);
    result.enemyName = enemy.getName();

    if (fightOneEnemy(player, enemy))
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
    }

    ConsoleUI::pause();
    return result;
}

// ============================================================
// 冒险战斗（根据阶段生成敌人，支持多敌人）
// ============================================================

BattleResult BattleSystem::startBattle(Character& player, int stage, bool isBoss)
{
    BattleResult result;
    result.outcome = BattleOutcome::PLAYER_DEAD;

    // 使用工厂根据阶段生成敌人列表
    std::vector<Enemy> enemies = enemyFactory.generateEnemies(stage);

    if (enemies.empty())
    {
        std::cout << "[BattleSystem] Error: No enemies generated for stage " << stage << "\n";
        return result;
    }

    // 显示遭遇信息
    ConsoleUI::printLine('-');
    ConsoleUI::setColor(isBoss ? GameConfig::COLOR_WARNING : GameConfig::COLOR_TITLE);
    std::cout << "\xe7\xac\xac " << stage << " \xe5\x9c\xba\xe6\x88\x98\xe6\x96\x97\xef\xbc\x8c\xe9\x81\xad\xe9\x81\x87 " << enemies.size() << " \xe4\xb8\xaa\xe6\x95\x8c\xe4\xba\xba\xef\xbc\x9a\n";
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        std::cout << "  " << (i + 1) << ". ";
        enemies[i].showInfo();
    }
    ConsoleUI::resetColor();

    if (!result.enemyName.empty())
        result.enemyName += ", ";
    for (const auto& e : enemies)
        result.enemyName += e.getName() + " ";

    // 逐一对战每个敌人
    int defeated = 0;
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        if (i > 0)
        {
            ConsoleUI::clearScreen();
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "\xe4\xb8\x8b\xe4\xb8\x80\xe4\xb8\xaa\xe6\x95\x8c\xe4\xba\xba\xef\xbc\x81\xe5\x89\xa9\xe4\xbd\x99 " << (enemies.size() - i) << " \xe4\xb8\xaa...\n";
            ConsoleUI::resetColor();
            ConsoleUI::pause();
        }

        if (fightOneEnemy(player, enemies[i]))
        {
            defeated++;
            player.gainExp(enemies[i].getRewardExp());
            player.gainGold(enemies[i].getRewardGold());
        }
        else
        {
            // 战败
            result.outcome = BattleOutcome::PLAYER_DEAD;
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "\xe8\xa2\xab " << enemies[i].getName() << " \xe5\x87\xbb\xe8\xb4\xa5\xef\xbc\x81\n";
            ConsoleUI::resetColor();

            // 恢复部分生命
            const int fallbackHp = std::max(1, player.getMaxHp() / 4);
            player.heal(fallbackHp);

            ConsoleUI::pause();
            return result;
        }
    }

    // 所有敌人击败
    result.outcome = BattleOutcome::PLAYER_WIN;
    ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
    std::cout << "\xe5\x87\xbb\xe8\xb4\xa5\xe4\xba\x86\xe6\x89\x80\xe6\x9c\x89 " << defeated << " \xe4\xb8\xaa\xe6\x95\x8c\xe4\xba\xba\xef\xbc\x81\n";
    ConsoleUI::resetColor();

    ConsoleUI::pause();
    return result;
}

// ============================================================
// 单敌人战斗循环
// ============================================================

bool BattleSystem::fightOneEnemy(Character& player, const Enemy& enemy) const
{
    Enemy enemyCopy = enemy;  // 可修改的副本
    const int enemyMaxHp = enemyCopy.getHp();

    while (player.isAlive() && enemyCopy.isAlive())
    {
        ConsoleUI::clearScreen();
        ConsoleUI::printTitle("\xe6\x88\x98\xe6\x96\x97");
        std::cout << player.getName() << " HP: " << player.getHp() << "/" << player.getMaxHp()
                  << " " << makeHpBar(player.getHp(), player.getMaxHp()) << "\n";
        std::cout << enemyCopy.getName() << " HP: " << enemyCopy.getHp() << "/" << enemyMaxHp
                  << " " << makeHpBar(enemyCopy.getHp(), enemyMaxHp) << "\n\n";

        std::cout << "1. \xe6\x94\xbb\xe5\x87\xbb\n";
        std::cout << "0. \xe9\x80\x83\xe8\xb7\x91\n";
        int confirm = ConsoleUI::readInt("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9: ");

        if (confirm == 0) return false;  // 逃跑
        if (confirm != 1) continue;

        const int playerDamage = calculateDamage(player.getAttack(), enemyCopy.getDefense());
        enemyCopy.takeDamage(playerDamage);
        std::cout << player.getName() << " \xe5\xaf\xb9 " << enemyCopy.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << playerDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        if (!enemyCopy.isAlive()) break;

        const int enemyDamage = calculateDamage(enemyCopy.getAttack(), player.getDefense());
        player.takeDamage(enemyDamage);
        std::cout << enemyCopy.getName() << " \xe5\xaf\xb9 " << player.getName()
                  << " \xe9\x80\xa0\xe6\x88\x90\xe4\xba\x86 " << enemyDamage << " \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3\xe3\x80\x82\n";

        ConsoleUI::pause();
    }

    return player.isAlive();
}

int BattleSystem::calculateDamage(int attack, int defense) const
{
    return std::max(1, attack - defense);
}
