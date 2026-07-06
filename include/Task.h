// Task.h
#ifndef TASK_H
#define TASK_H

#include "Character.h"

#include <string>

// ============================================================
// TaskStatus
// ------------------------------------------------------------
// 任务状态枚举。
// ============================================================

enum class TaskStatus
{
    NotAccepted,
    Accepted,
    Completed,
    RewardClaimed
};

// ============================================================
// Task
// ------------------------------------------------------------
// 任务类。
// 包含任务名称、描述、完成条件、奖励和当前状态。
// ============================================================

class Task
{
private:
    std::string name;
    std::string description;
    std::string condition;
    int rewardExp;
    int rewardGold;
    TaskStatus status;

public:
    Task();
    Task(
        const std::string& name,
        const std::string& description,
        const std::string& condition,
        int rewardExp,
        int rewardGold
    );

    // 显示任务详细信息。
    void showInfo() const;

    // 接受任务。
    void accept();

    // 标记任务完成。
    void complete();

    // 领取任务奖励。
    void claimReward(Character& player);

    TaskStatus getStatus() const;
    std::string getName() const;
};

#endif