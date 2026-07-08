// Task.h
#ifndef TASK_H
#define TASK_H

#include "Character.h"

#include <string>
#include <vector>
#include <iostream>

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
// TaskMessage
// ------------------------------------------------------------
// 任务事件消息，用于驱动任务进度自动更新。
// type: 事件类型 (kill / collect / gold / battle_win 等)
// target: 事件目标名称
// value: 事件数值
// ============================================================

struct TaskMessage
{
    std::string type;
    std::string target;
    int value;
};

// ============================================================
// Objective
// ------------------------------------------------------------
// 任务的子目标。每个任务可包含多个子目标。
// type:    匹配的事件类型
// target:  匹配的事件目标名称
// required: 需要完成的数量
// current:  当前已完成的进度
// description: 目标描述文本
// ============================================================

struct Objective
{
    std::string type;
    std::string target;
    int required;
    int current;
    std::string description;
};

// ============================================================
// Task
// ------------------------------------------------------------
// 任务类。
// 包含任务名称、描述、多子目标、奖励和当前状态。
// 支持通过事件消息自动推进进度。
// ============================================================

class Task
{
    friend class SaveManager;

private:
    std::string name;
    std::string description;
    std::vector<Objective> objectives;
    int rewardExp;
    int rewardGold;
    TaskStatus status;

public:
    Task();
    Task(
        const std::string& name,
        const std::string& description,
        const std::vector<Objective>& objectives,
        int rewardExp,
        int rewardGold
    );

    // 显示任务详细信息（含各子目标进度）
    void showInfo() const;

    // 接受任务
    void accept();

    // 手动标记任务完成
    void complete();

    // 领取任务奖励
    void claimReward(Character& player);

    // ---- 新增：事件驱动进度更新 ----

    // 处理一条任务消息，匹配子目标并推进进度
    // 返回 true 表示有目标进度被更新
    bool onMessage(const TaskMessage& msg);

    // 检查所有子目标是否已完成
    bool isAllObjectivesComplete() const;

    // 检查并自动完成任务（当所有目标完成时自动切换状态并打印日志）
    bool checkAndAutoComplete();

    // 获取子目标列表（只读）
    const std::vector<Objective>& getObjectives() const { return objectives; }

    // 重置所有子目标进度（用于任务放弃等场景）
    void resetProgress();

    TaskStatus getStatus() const;
    std::string getName() const;
};

#endif
