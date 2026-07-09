#include "BattleAI.h"
#include "PathUtil.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
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
    // Python 侧只需要一个极简牌面列表，不复用 C++ 结构体序列化，
    // 这样脚本更容易独立调试。
    std::ostringstream oss;
    for (size_t i = 0; i < hand.size(); ++i)
    {
        if (i > 0) oss << ",";
        oss << hand[i].getActionId();
    }
    return oss.str();
}

int estimateAttackDamage(const BattleActorState& attacker, const BattleActorState& defender)
{
    // AI 评估时复用和正式战斗接近的伤害估算，
    // 但不把瞬时防御倍率带进来，避免决策依赖尚未结算的回合状态。
    int damage = std::max(1, attacker.attack - defender.defense);
    if (hasTypeAdvantage(attacker.element, defender.element))
        damage *= 2;
    damage = static_cast<int>(damage * (1.0 + 0.5 * defender.vulnerabilityStacks));
    return std::max(1, damage);
}
}

BattleDecision BattleAI::decideEnemyAction(
    const BattleTurnContext& context,
    const BattleActorState& player,
    const BattleActorState& enemy)
{
    // Python 有结果就信任 Python；否则立即落回本地策略。
    BattleDecision pythonDecision = tryPythonDecision(context, player, enemy);
    if (pythonDecision.action != BattleAction::None)
        return pythonDecision;

    return localFallbackDecision(context, player, enemy);
}

BattleDecision BattleAI::tryPythonDecision(
    const BattleTurnContext& context,
    const BattleActorState& player,
    const BattleActorState& enemy)
{
    // Python AI 是“增强层”，找不到脚本时直接交给本地策略，
    // 不把外部依赖扩散到主战斗流程。
    std::string scriptPath = PathUtil::resolvePath("scripts/battle_ai.py");
    if (scriptPath.empty())
        return {};

    std::ostringstream command;
    // 命令行参数保持扁平，避免为了一个可选 AI 层引入额外 IPC 协议。
    command << "python \"" << scriptPath << "\" "
            << context.round << " "
            << (context.playerFirst ? 1 : 0) << " "
            << static_cast<int>(context.revealedPlayerAction) << " "
            << player.currentHp << " "
            << player.maxHp << " "
            << player.attack << " "
            << static_cast<int>(player.element) << " "
            << enemy.currentHp << " "
            << enemy.maxHp << " "
            << enemy.attack << " "
            << static_cast<int>(enemy.element) << " "
            << "\"" << buildAvailableCards(context.enemyHand) << "\" "
            << (enemy.commandUsed ? 1 : 0) << " "
            << (enemy.disarmed ? 1 : 0)
            << " 2>&1";

    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(command.str().c_str(), "r"), PCLOSE);
    if (!pipe)
        return {};

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        const std::string chunk = buffer.data();
        result += chunk;
        std::cout << chunk << std::flush;
    }

    try
    {
        // Python 日志和分析文本会一起输出到控制台，因此这里从后向前提取最后一个动作数字。
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
        if (actionValue == 1) action = BattleAction::Attack;
        else if (actionValue == 2) action = BattleAction::Defense;
        else if (actionValue == 3) action = BattleAction::Heal;
        if (action == BattleAction::None)
            return {};
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
    const bool canUseCommand = !enemy.commandUsed;

    // 已经缴械的回合里，攻击一定会落空，优先考虑回复或防御。
    if (enemy.disarmed)
    {
        if (myHpRatio < 0.8 && (hasHeal || canUseCommand))
            return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
        if (hasDefense || canUseCommand)
            return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);
    }

    if (context.playerFirst && context.revealedPlayerAction != BattleAction::None)
    {
        // 后手 AI 可以利用“公开意图”做针对性应对，这是这套战斗系统的核心博弈点。
        if (context.revealedPlayerAction == BattleAction::Attack && (hasDefense || canUseCommand))
            return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);

        if (context.revealedPlayerAction == BattleAction::Defense)
        {
            if ((hpGap >= healAmount && hasHeal) || (hasHeal && myHpRatio < 0.9) || canUseCommand)
                return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
            return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
        }

        if (context.revealedPlayerAction == BattleAction::Heal && (hasAttack || canUseCommand))
            return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    }

    if (myHpRatio < 0.25 && (hasHeal || canUseCommand))
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);

    if (player.currentHp <= myAttackDamage && (hasAttack || canUseCommand))
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);

    if (hasTypeAdvantage(enemy.element, player.element) && myHpRatio >= 0.4 && (hasAttack || canUseCommand))
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);

    if (hasTypeAdvantage(player.element, enemy.element) && myHpRatio < 0.5 && (hasDefense || canUseCommand))
        return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);

    if (myHpRatio >= 0.25 && myHpRatio < 0.6 && hpGap >= healAmount && (hasHeal || canUseCommand))
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);

    if (enemyHpRatio < 0.4 && (hasAttack || canUseCommand))
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);

    if (myHpRatio >= 0.6 && (hasAttack || canUseCommand))
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);

    if (hasAttack || canUseCommand)
        return materializeDecision(BattleAction::Attack, context.enemyHand, enemy.commandUsed);
    if (hasHeal)
        return materializeDecision(BattleAction::Heal, context.enemyHand, enemy.commandUsed);
    if (hasDefense)
        return materializeDecision(BattleAction::Defense, context.enemyHand, enemy.commandUsed);

    return {};
}

BattleDecision BattleAI::materializeDecision(
    BattleAction desiredAction,
    const std::vector<BattleCard>& hand,
    bool commandUsed)
{
    CardType desiredType = CardType::Attack;
    if (desiredAction == BattleAction::Defense) desiredType = CardType::Defense;
    else if (desiredAction == BattleAction::Heal) desiredType = CardType::Heal;

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

    if (!commandUsed && desiredAction != BattleAction::None)
    {
        BattleDecision decision;
        decision.action = desiredAction;
        decision.usedCommand = true;
        // 这里不指定 selectedIndex，因为指令不是从当前手牌槽位打出的。
        return decision;
    }

    return {};
}
