#include "BattleTypes.h"

std::string BattleCard::getName() const
{
    return toString(type);
}

int BattleCard::getActionId() const
{
    return static_cast<int>(type);
}

std::string toString(BattleAction action)
{
    switch (action)
    {
    case BattleAction::Attack: return "攻击";
    case BattleAction::Defense: return "防御";
    case BattleAction::Heal: return "回复";
    case BattleAction::Escape: return "逃跑";
    case BattleAction::UseItem: return "使用道具";
    default: return "无";
    }
}

std::string toString(CardType type)
{
    switch (type)
    {
    case CardType::Attack: return "攻击牌";
    case CardType::Defense: return "防御牌";
    case CardType::Heal: return "回复牌";
    default: return "未知牌";
    }
}
