#ifndef BATTLE_AI_H
#define BATTLE_AI_H

#include "BattleTypes.h"

#include <vector>

class BattleAI
{
public:
    // 优先尝试 Python AI；失败时必须无缝回退到本地策略，
    // 不能让战斗流程依赖外部脚本或网络环境。
    static BattleDecision decideEnemyAction(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

private:
    // Python AI 只负责给出动作意图，不直接修改战斗状态。
    static BattleDecision tryPythonDecision(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

    // 本地策略必须完整可用，保证离线时战斗仍可运行。
    static BattleDecision localFallbackDecision(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

    // 敌方 AI 只能从当前手牌中选择真实存在的动作，不能使用指令越权出牌。
    static BattleDecision materializeDecision(
        BattleAction desiredAction,
        const std::vector<BattleCard>& hand,
        bool commandUsed);
};

#endif
