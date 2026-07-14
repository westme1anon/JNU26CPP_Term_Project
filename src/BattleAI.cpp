#include "BattleAI.h"
#include "PathUtil.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace
{
bool hasCardType(const std::vector<BattleCard>& hand, CardType type)
{
    return std::any_of(hand.begin(), hand.end(), [type](const BattleCard& card) {
        return card.type == type;
    });
}

std::string buildAvailableCards(const std::vector<BattleCard>& hand)
{
    std::ostringstream oss;
    for (size_t i = 0; i < hand.size(); ++i)
    {
        if (i > 0)
        {
            oss << ",";
        }
        oss << hand[i].getActionId();
    }
    return oss.str();
}

int estimateAttackDamage(const BattleActorState& attacker, const BattleActorState& defender)
{
    int damage = std::max(1, attacker.attack - defender.defense);
    damage = static_cast<int>(damage * (1.0 + 0.5 * defender.vulnerabilityStacks));
    return std::max(1, damage);
}
}

BattleDecision BattleAI::decideEnemyAction(
    const BattleTurnContext& context,
    const BattleActorState& player,
    const BattleActorState& enemy)
{
    BattleDecision pythonDecision = tryPythonDecision(context, player, enemy);
    if (pythonDecision.action != BattleAction::None)
    {
        return pythonDecision;
    }

    return localFallbackDecision(context, player, enemy);
}

BattleDecision BattleAI::tryPythonDecision(
    const BattleTurnContext& context,
    const BattleActorState& player,
    const BattleActorState& enemy)
{
    std::string scriptPath = PathUtil::resolvePath("scripts/battle_ai.py");
    if (scriptPath.empty())
    {
        return {};
    }

    std::ostringstream command;
    command << "python \"" << scriptPath << "\" "
            << context.round << " "
            << (context.playerFirst ? 1 : 0) << " "
            << static_cast<int>(context.revealedPlayerAction) << " "
            << player.currentHp << " "
            << player.maxHp << " "
            << player.attack << " "
            << player.defense << " "
            << player.vulnerabilityStacks << " "
            << "\"" << buildAvailableCards(context.playerHand) << "\" "
            << (player.commandUsed ? 1 : 0) << " "
            << enemy.currentHp << " "
            << enemy.maxHp << " "
            << enemy.attack << " "
            << enemy.defense << " "
            << enemy.vulnerabilityStacks << " "
            << "\"" << buildAvailableCards(context.enemyHand) << "\" "
            << (enemy.commandUsed ? 1 : 0) << " "
            << (enemy.disarmed ? 1 : 0)
            << " 2>&1";

    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(command.str().c_str(), "r"), PCLOSE);
    if (!pipe)
    {
        return {};
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        const std::string chunk = buffer.data();
        result += chunk;
        std::cout << chunk << std::flush;
    }

    try
    {
        int actionValue = 0;
        for (auto it = result.rbegin(); it != result.rend(); ++it)
        {
            if (*it == '1' || *it == '2' || *it == '3')
            {
                actionValue = *it - '0';
                break;
            }
        }

        BattleAction action = BattleAction::None;
        if (actionValue == 1)
        {
            action = BattleAction::Attack;
        }
        else if (actionValue == 2)
        {
            action = BattleAction::Defense;
        }
        else if (actionValue == 3)
        {
            action = BattleAction::Heal;
        }

        if (action == BattleAction::None)
        {
            return {};
        }
        return materializeDecision(action, context.enemyHand, enemy.commandUsed);
    }
    catch (...)
    {
        return {};
    }
}

BattleDecision BattleAI::localFallbackDecision(
    const BattleTurnContext& context,
    const BattleActorState& player,
    const BattleActorState& enemy)
{
    const bool hasAttack = hasCardType(context.enemyHand, CardType::Attack);
    const bool hasDefense = hasCardType(context.enemyHand, CardType::Defense);
    const bool hasHeal = hasCardType(context.enemyHand, CardType::Heal);

    const int myAttackDamage = estimateAttackDamage(enemy, player);
    const double myHpRatio = (enemy.maxHp > 0) ? static_cast<double>(enemy.currentHp) / enemy.maxHp : 0.0;
    const double enemyHpRatio = (player.maxHp > 0) ? static_cast<double>(player.currentHp) / player.maxHp : 0.0;
    const int healAmount = std::max(1, enemy.attack * 8 / 10);
    const int hpGap = enemy.maxHp - enemy.currentHp;

    if (enemy.disarmed)
    {
        if (myHpRatio < 0.8 && hasHeal)
        {
            return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
        }
        if (hasDefense)
        {
            return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);
        }
    }

    if (context.playerFirst && context.revealedPlayerAction != BattleAction::None)
    {
        if (context.revealedPlayerAction == BattleAction::Attack && hasDefense)
        {
            return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);
        }

        if (context.revealedPlayerAction == BattleAction::Defense)
        {
            if ((hpGap >= healAmount && hasHeal) || (hasHeal && myHpRatio < 0.9))
            {
                return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
            }
            return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
        }

        if (context.revealedPlayerAction == BattleAction::Heal && hasAttack)
        {
            return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
        }
    }

    if (myHpRatio < 0.25 && hasHeal)
    {
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
    }

    if (player.currentHp <= myAttackDamage && hasAttack)
    {
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    }

    if (myHpRatio >= 0.25 && myHpRatio < 0.6 && hpGap >= healAmount && hasHeal)
    {
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
    }

    if (enemyHpRatio < 0.4 && hasAttack)
    {
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    }

    if (myHpRatio >= 0.6 && hasAttack)
    {
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    }

    if (hasAttack)
    {
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    }
    if (hasHeal)
    {
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
    }
    if (hasDefense)
    {
        return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);
    }

    return {};
}

BattleDecision BattleAI::materializeDecision(
    BattleAction desiredAction,
    const std::vector<BattleCard>& hand,
    bool commandUsed)
{
    (void)commandUsed;

    CardType desiredType = CardType::Attack;
    if (desiredAction == BattleAction::Defense)
    {
        desiredType = CardType::Defense;
    }
    else if (desiredAction == BattleAction::Heal)
    {
        desiredType = CardType::Heal;
    }

    for (size_t i = 0; i < hand.size(); ++i)
    {
        if (hand[i].type == desiredType)
        {
            BattleDecision decision;
            decision.action = desiredAction;
            decision.selectedIndex = static_cast<int>(i);
            return decision;
        }
    }

    return {};
}
