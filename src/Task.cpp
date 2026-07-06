// Task.cpp
#include "Task.h"

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
    // TODO: 输出任务名称、描述、条件、奖励、当前状态。
}

void Task::accept()
{
    // TODO: 如果任务未接受，则修改状态为 Accepted。
}

void Task::complete()
{
    // TODO: 如果任务已接受，则修改状态为 Completed。
}

void Task::claimReward(Character& player)
{
    // TODO: 如果任务已完成，则给玩家经验和金币，并改为 RewardClaimed。
}

TaskStatus Task::getStatus() const
{
    return status;
}

std::string Task::getName() const
{
    return name;
}