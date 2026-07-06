// AIAssistant.h
#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include "Character.h"
#include "Inventory.h"

#include <string>
#include <unordered_map>

// ============================================================
// AIAssistant
// ------------------------------------------------------------
// 游戏 AI 助手。
// 当前阶段先使用规则系统实现智能建议。
// 后续可扩展为大模型 API 调用或复杂决策系统。
// ============================================================

class AIAssistant
{
private:
    std::unordered_map<std::string, std::string> tips;

public:
    AIAssistant();

    // 根据玩家当前状态显示提示。
    void showTips(const Character& player) const;

    // 根据玩家状态和背包情况推荐下一步行动。
    std::string suggestAction(const Character& player, const Inventory& inventory) const;

    // 初始化内置提示规则。
    void initTips();
};

#endif