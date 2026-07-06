// TaskSystem.h
#ifndef TASK_SYSTEM_H
#define TASK_SYSTEM_H

#include "Task.h"

#include <vector>

// ============================================================
// TaskSystem
// ------------------------------------------------------------
// 任务系统。
// 管理所有任务，支持查看、接受、完成和领取奖励。
// ============================================================

class TaskSystem
{
private:
    std::vector<Task> tasks;

public:
    TaskSystem();

    // 加载任务数据。
    void loadTasks();

    // 显示所有任务。
    void showTasks() const;

    // 接受指定任务。
    bool acceptTask(int index);

    // 完成指定任务。
    bool completeTask(int index);

    // 领取指定任务奖励。
    bool claimReward(int index, Character& player);

    // 获取任务数量。
    int size() const;
};

#endif