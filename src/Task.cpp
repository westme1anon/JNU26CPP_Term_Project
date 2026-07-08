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
      status(TaskStatus::NotAccepted)
{
}

Task::Task(
    const std::string& name,
    const std::string& description,
    const std::vector<Objective>& objectives,
    int rewardExp,
    int rewardGold
)
    : name(name),
      description(description),
      objectives(objectives),
      rewardExp(rewardExp),
      rewardGold(rewardGold),
      status(TaskStatus::NotAccepted)
{
}

void Task::showInfo() const
{
    ConsoleUI::printLine('-', 60);
    std::cout << "任务名称: " << name << "\n";
    std::cout << "任务描述: " << description << "\n";

    // 显示子目标及进度
    if (!objectives.empty())
    {
        std::cout << "任务目标:\n";
        for (size_t i = 0; i < objectives.size(); ++i)
        {
            const auto& obj = objectives[i];
            std::cout << "  [" << (i + 1) << "] " << obj.description
                      << " (" << obj.current << "/" << obj.required << ")";
            if (obj.current >= obj.required)
            {
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
    std::cout << "\n";

    std::cout << "当前状态: ";
    switch (status)
    {
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
    if (status == TaskStatus::NotAccepted)
    {
        status = TaskStatus::Accepted;
        // 重置进度
        for (auto& obj : objectives)
            obj.current = 0;

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 成功接受任务: " << name << "\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 无法接受（当前状态不允许）。\n";
        ConsoleUI::resetColor();
    }
}

void Task::complete()
{
    if (status == TaskStatus::Accepted)
    {
        status = TaskStatus::Completed;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 任务完成: " << name << "\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 无法完成（请先接受任务）。\n";
        ConsoleUI::resetColor();
    }
}

void Task::claimReward(Character& player)
{
    if (status == TaskStatus::Completed)
    {
        player.gainExp(rewardExp);
        player.gainGold(rewardGold);

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[任务] 领取任务奖励: " << name << "\n";
        std::cout << "  获得 " << rewardExp << " 经验值，" << rewardGold << " 金币。\n";
        ConsoleUI::resetColor();

        status = TaskStatus::RewardClaimed;
    }
    else if (status == TaskStatus::RewardClaimed)
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 的奖励已经领取过了。\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "[任务] " << name << " 尚未完成，无法领取奖励。\n";
        ConsoleUI::resetColor();
    }
}

// ============================================================
// 事件驱动进度更新实现
// ============================================================

bool Task::onMessage(const TaskMessage& msg)
{
    // 仅处理已接受且未完成的任务
    if (status != TaskStatus::Accepted)
        return false;

    bool updated = false;

    for (auto& obj : objectives)
    {
        // 已完成的子目标跳过
        if (obj.current >= obj.required)
            continue;

        // 匹配事件类型
        if (obj.type != msg.type)
            continue;

        // 如果目标指定了 target 名称，则需要匹配（空字符串表示匹配任意）
        if (!obj.target.empty() && obj.target != msg.target)
            continue;

        // 推进进度
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

    for (const auto& obj : objectives)
    {
        if (obj.current < obj.required)
            return false;
    }
    return true;
}

bool Task::checkAndAutoComplete()
{
    if (status != TaskStatus::Accepted)
        return false;

    if (isAllObjectivesComplete())
    {
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
