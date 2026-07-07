// Task.cpp
#include "Task.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>

Task::Task()
    : name("未知任务"),
      description("暂无描述"),
      condition("暂无条件"),
      rewardExp(0),
      rewardGold(0),
      status(TaskStatus::NotAccepted)
{
}

Task::Task(
    const std::string& name,
    const std::string& description,
    const std::string& condition,
    int rewardExp,
    int rewardGold
)
    : name(name),
      description(description),
      condition(condition),
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
    std::cout << "完成条件: " << condition << "\n";

    std::cout << "任务奖励: ";

    if (rewardExp > 0)
    {
        std::cout << rewardExp << " 经验";
    }
    if (rewardExp > 0 && rewardGold > 0)
    {
        std::cout << "，";
    }
    if (rewardGold > 0)
    {
        std::cout << rewardGold << " 金币";
    }
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
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "成功接受任务: " << name << "\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "任务 " << name << " 无法接受（当前状态不允许）。\n";
        ConsoleUI::resetColor();
    }
}

void Task::complete()
{
    if (status == TaskStatus::Accepted)
    {
        status = TaskStatus::Completed;
        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "任务完成: " << name << "\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "任务 " << name << " 无法完成（请先接受任务）。\n";
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
        std::cout << "领取任务奖励: " << name << "\n";
        std::cout << "获得 " << rewardExp << " 经验值，"
                  << rewardGold << " 金币。\n";
        ConsoleUI::resetColor();

        status = TaskStatus::RewardClaimed;
    }
    else if (status == TaskStatus::RewardClaimed)
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "任务 " << name << " 的奖励已经领取过了。\n";
        ConsoleUI::resetColor();
    }
    else
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "任务 " << name << " 尚未完成，无法领取奖励。\n";
        ConsoleUI::resetColor();
    }
}

TaskStatus Task::getStatus() const
{
    return status;
}

std::string Task::getName() const
{
    return name;
}
