#include "BattleMapper.h"
#include "Character.h"
#include "Enemy.h"

#include <algorithm>

namespace BattleMapper
{
namespace
{
constexpr int kPlayerBattleHpNumerator = 3;
constexpr int kPlayerBattleHpDenominator = 4;
constexpr int kEnemyBattleHpNumerator = 3;
constexpr int kEnemyBattleHpDenominator = 4;
}

BattleActorState fromCharacter(const Character& player)
{
    BattleActorState state;
    state.name = player.getName();
    const int sourceMaxHp = std::max(1, player.getMaxHp());
    state.maxHp = std::max(30, sourceMaxHp * kPlayerBattleHpNumerator / kPlayerBattleHpDenominator);
    state.currentHp = (player.getHp() > 0)
        ? std::max(1, player.getHp() * state.maxHp / sourceMaxHp)
        : 0;
    state.attack = player.getAttack();
    state.defense = player.getDefense();
    state.speed = std::max(1, player.getLevel() + player.getAttack() / 5);
    return state;
}

BattleActorState fromEnemy(const Enemy& enemy)
{
    BattleActorState state;
    state.name = enemy.getName();

    int mappedHp = enemy.getHp();
    int mappedAttack = enemy.getAttack();
    int mappedDefense = enemy.getDefense();

    switch (enemy.getTier())
    {
    case EnemyTier::Minion:
        mappedHp = std::max(35, enemy.getHp() * 4 / 5);
        mappedAttack = std::max(8, enemy.getAttack() * 9 / 10);
        mappedDefense = std::max(1, enemy.getDefense() * 2 / 3);
        break;

    case EnemyTier::Elite:
        mappedHp = std::max(55, enemy.getHp() * 3 / 4);
        mappedAttack = std::max(12, enemy.getAttack() * 17 / 20);
        mappedDefense = std::max(2, enemy.getDefense() * 3 / 5);
        break;

    case EnemyTier::Boss:
        mappedHp = std::max(90, enemy.getHp() * 2 / 3);
        mappedAttack = std::max(16, enemy.getAttack() * 4 / 5);
        mappedDefense = std::max(3, enemy.getDefense() / 2);
        break;
    }

    mappedDefense = std::min(mappedDefense, std::max(3, mappedAttack / 2));

    mappedHp = std::max(24, mappedHp * kEnemyBattleHpNumerator / kEnemyBattleHpDenominator);

    state.currentHp = mappedHp;
    state.maxHp = mappedHp;
    state.attack = mappedAttack;
    state.defense = mappedDefense;
    state.speed = enemy.getSpd();
    return state;
}

void applyPlayerHp(Character& player, int newHp)
{
    newHp = std::max(0, std::min(newHp, player.getMaxHp()));
    const int currentHp = player.getHp();
    if (newHp < currentHp)
        player.takeDamage(currentHp - newHp);
    else if (newHp > currentHp)
        player.heal(newHp - currentHp);
}

void applyPlayerHp(Character& player, int newHp, int battleMaxHp)
{
    const int safeBattleMaxHp = std::max(1, battleMaxHp);
    const int clampedBattleHp = std::max(0, std::min(newHp, safeBattleMaxHp));
    const int targetHp = (clampedBattleHp > 0)
        ? std::max(1, std::min(player.getMaxHp(), clampedBattleHp * player.getMaxHp() / safeBattleMaxHp))
        : 0;
    applyPlayerHp(player, targetHp);
}
}
