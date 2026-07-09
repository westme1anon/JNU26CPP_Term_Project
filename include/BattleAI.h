#ifndef BATTLE_AI_H
#define BATTLE_AI_H

#include "BattleTypes.h"

#include <vector>

class BattleAI
{
public:
    // 先尝试 Python AI；失败时必须无缝降级到本地策略，
    // 不能让战斗流程依赖外部脚本或网络环境。
    static BattleDecision decideEnemyAction(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

private:
    // Python AI 只负责“给出一个动作意图”，不直接控制主循环或修改战斗状态。
    static BattleDecision tryPythonDecision(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

    // 本地策略必须完整可用，确保离线、无 Python、无网络时战斗仍可闭环。
    static BattleDecision localFallbackDecision(
        const BattleTurnContext& context,
        const BattleActorState& player,
        const BattleActorState& enemy);

    // 把“想执行的动作”映射为最终可执行决策：
    // 优先消耗手牌；若手牌没有且指令未用，则自动转成一次指令越权出牌。
    static BattleDecision materializeDecision(
        BattleAction desiredAction,
        const std::vector<BattleCard>& hand,
        bool commandUsed);
};

#endif
