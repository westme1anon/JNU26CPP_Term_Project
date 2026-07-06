// TaskSystem.cpp
#include "TaskSystem.h"

#include <iostream>

TaskSystem::TaskSystem()
{
    loadTasks();
}

void TaskSystem::loadTasks()
{
    // TODO: 从 data/tasks.txt 加载任务。
    // 临时可初始化三个校园任务：
    // 1. 新生报到
    // 2. 图书馆探索
    // 3. 击败考试魔王
}

void TaskSystem::showTasks() const
{
    // TODO: 遍历 tasks 并调用 Task::showInfo。
}

bool TaskSystem::acceptTask(int index)
{
    // TODO: 校验 index，调用对应任务 accept。
    return false;
}

bool TaskSystem::completeTask(int index)
{
    // TODO: 校验 index，调用对应任务 complete。
    return false;
}

bool TaskSystem::claimReward(int index, Character& player)
{
    // TODO: 校验 index，调用对应任务 claimReward。
    return false;
}

int TaskSystem::size() const
{
    return static_cast<int>(tasks.size());
}