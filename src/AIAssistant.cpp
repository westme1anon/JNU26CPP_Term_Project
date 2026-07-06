// AIAssistant.cpp
#include "AIAssistant.h"

#include <iostream>

AIAssistant::AIAssistant()
{
    initTips();
}

void AIAssistant::initTips()
{
    // TODO: 初始化规则提示。
    // 例如：低血量建议使用药品，金币多建议去商店，等级低建议刷怪。
}

void AIAssistant::showTips(const Character& player) const
{
    // TODO: 根据角色等级、血量、金币等输出提示。
}

std::string AIAssistant::suggestAction(const Character& player, const Inventory& inventory) const
{
    // TODO:
    // 1. 如果 HP 较低，建议使用药品或食物
    // 2. 如果金币较多，建议购买装备
    // 3. 如果等级较低，建议挑战低级敌人
    return "建议先查看角色状态，并根据当前生命值决定是否进入战斗。";
}