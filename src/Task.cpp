// Task.cpp
#include "Task.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>
#include <algorithm>

Task::Task()
    : name("未知任务"),
      description("暂无描述"),
      rewardExp(0),
      rewardGold(0),
      status(TaskStatus::NotAccepted),
      questType(QuestType::World)
{
}

Task::Task(
    const std::string& name,
    const std::string& description,
    const std::vector<Objective>& objectives,
    int rewardExp,
    int rewardGold,
    QuestType questType,
    const std::string& questId,
    const std::string& prerequisiteQuestId
)
    : name(name),
      description(description),
      objectives(objectives),
      rewardExp(rewardExp),
      rewardGold(rewardGold),
      status(TaskStatus::NotAccepted),
      questType(questType),
      questId(questId),
      prerequisiteQuestId(prerequisiteQuestId)
{
}

void Task::showInfo() const
{
    ConsoleUI::printLine('-', 60);
    std::cout << "[类型] ";
    switch (questType) {
    case QuestType::Main:
        ConsoleUI::setColor(GameConfig::COLOR_TITLE);
        std::cout << "主线任务";
        break;
    case QuestType::World:
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "世界任务";
        break;
    case QuestType::Hidden:
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "隐藏任务";
        break;
    }
    ConsoleUI::resetColor();
    std::cout << "\n";

    if (!questId.empty())
        std::cout << "[ID] " << questId << "\n";

    std::cout << "任务名称: " << name << "\n";
    std::cout << "任务描述: " << description << "\n";

    // 前置条件
    if (!prerequisiteQuestId.empty())
        std::cout << "[前置任务] " << prerequisiteQuestId << "\n";
    if (minLevel > 1)
        std::cout << "[等级要求] >= " << minLevel << " 级\n";

    // 子目标
    if (!objectives.empty()) {
        std::cout << "任务目标:\n";
        for (size_t i = 0; i < objectives.size(); ++i) {
            const auto& obj = objectives[i];
            std::cout << "  [" << (i + 1) << "] " << obj.description
                      << " (" << obj.current << "/" << obj.required << ")";
            if (obj.current >= obj.required) {
                ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
                std::cout << " [已完成]";
                ConsoleUI::resetColor();
            }
            std::cout << "\n";
        }
    }

    std::cout << "任务奖励: ";
    if (rewardExp > 0) std::cout << rewardExp << " 经验";
    if (rewardExp > 0 && rewardGold > 0) std::cout << "，";
    if (rewardGold > 0) std::cout << rewardGold << " 金币";
    if (onCompleteUnlockShop) std::cout << " | 解锁商店";
    std::cout << "\n";

    std::cout << "当前状态: ";
    switch (status) {
    case TaskStatus::NotAccepted:
        ConsoleUI::setColor(GameConfig::COLOR_DEFAULT);
        std::cout << "未接受";
        ConsoleUI::resetColor();
        break;
    case TaskStatus::Accepted:
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "已接受（进行中）";
        ConsoleUI::resetColor();
        break;
    case TaskStatus::Completed:
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "已完成（可领奖）";
        ConsoleUI::resetColor();
        break;
    case TaskStatus::RewardClaimed:
        ConsoleUI::setColor(GameConfig::COLOR_TITLE);
        std::cout << "奖励已领取";
        ConsoleUI::resetColor();
        break;
    }
    std::cout << "\n";
    ConsoleUI::printLine('-', 60);
}

void Task::accept()
{
    if (status == TaskStatus::NotAccepted) {
        status = TaskStatus::Accepted;
        for (auto& obj : objectives)
            obj.current = 0;

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 成功接受任务: " << name << "\n";
        ConsoleUI::resetColor();
    } else {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 无法接受（当前状态不允许）。\n";
        ConsoleUI::resetColor();
    }
}

void Task::complete()
{
    if (status == TaskStatus::Accepted) {
        status = TaskStatus::Completed;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 任务完成: " << name << "\n";
        ConsoleUI::resetColor();
    } else {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 无法完成（请先接受任务）。\n";
        ConsoleUI::resetColor();
    }
}

void Task::claimReward(Character& player)
{
    if (status == TaskStatus::Completed) {
        player.gainExp(rewardExp);
        player.gainGold(rewardGold);

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 领取任务奖励: " << name << "\n";
        std::cout << "  获得 " << rewardExp << " 经验值，" << rewardGold << " 金币。\n";
        if (onCompleteUnlockShop)
            std::cout << "  >> 校园商店已解锁！\n";
        ConsoleUI::resetColor();

        status = TaskStatus::RewardClaimed;
    } else if (status == TaskStatus::RewardClaimed) {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 的奖励已经领取过了。\n";
        ConsoleUI::resetColor();
    } else {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 尚未完成，无法领取奖励。\n";
        ConsoleUI::resetColor();
    }
}

// ============================================================
// 事件驱动进度更新
// ============================================================

bool Task::onMessage(const TaskMessage& msg)
{
    if (status != TaskStatus::Accepted)
        return false;

    bool updated = false;

    for (auto& obj : objectives) {
        if (obj.current >= obj.required)
            continue;
        if (obj.type != msg.type)
            continue;
        if (!obj.target.empty() && obj.target != msg.target)
            continue;

        int oldCurrent = obj.current;
        obj.current += msg.value;
        if (obj.current > obj.required)
            obj.current = obj.required;

        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务进度] " << name << " - " << obj.description
                  << " (" << oldCurrent << " -> " << obj.current << "/" << obj.required << ")\n";
        ConsoleUI::resetColor();

        updated = true;
    }

    return updated;
}

bool Task::isAllObjectivesComplete() const
{
    if (objectives.empty())
        return false;
    for (const auto& obj : objectives) {
        if (obj.current < obj.required)
            return false;
    }
    return true;
}

bool Task::checkAndAutoComplete()
{
    if (status != TaskStatus::Accepted)
        return false;
    if (isAllObjectivesComplete()) {
        status = TaskStatus::Completed;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "\n============================================================\n";
        std::cout << "[任务完成] " << name << " - 所有目标已达成！\n";
        std::cout << "  请前往任务菜单领取奖励。\n";
        std::cout << "============================================================\n\n";
        ConsoleUI::resetColor();
        return true;
    }
    return false;
}

void Task::resetProgress()
{
    for (auto& obj : objectives)
        obj.current = 0;
}

TaskStatus Task::getStatus() const
{
    return status;
}

std::string Task::getName() const
{
    return name;
}
