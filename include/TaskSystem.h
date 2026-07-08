// TaskSystem.h
#ifndef TASK_SYSTEM_H_TASK_SYSTEM_H
#define TASK_SYSTEM_H_TASK_SYSTEM_H

#include "Task.h"

#include <vector>

// ============================================================
// TaskSystem
// ------------------------------------------------------------
// 任务系统。
// 管理所有任务，支持按类型（主线/世界/隐藏）分类显示、
// 前置条件校验、隐藏任务触发、事件消息广播等。
// ============================================================

class TaskSystem
{
private:
    std::vector<Task> tasks;

    void loadLegacyTasks();

public:
    friend class SaveManager;

    TaskSystem();

    // JSON 加载
    void loadTasks();

    // ---- 自动接取默认任务 ----
    // 遍历所有任务，将 defaultAccepted=true 且未接取的任务自动接取
    void autoAcceptDefaults();

    // ---- 分类显示 ----
    void showTasksOverview() const;
    void showTasksByType(QuestType type, bool showHidden = false) const;

    // ---- 条件校验 ----
    // 检查指定任务是否满足接取条件（等级、前置任务）
    bool canAcceptTask(int index, const Character& player) const;

    // ---- 接受任务（带校验 + 玩家上下文） ----
    bool acceptTask(int index, const Character& player);

    // ---- 手动完成 ----
    bool completeTask(int index);

    // ---- 领取奖励 ----
    // 返回 true 表示成功领取；若任务解锁了商店回调应由此触发
    bool claimReward(int index, Character& player);

    // ---- 隐藏任务触发 ----
    // 将隐藏任务设为可见并接取（用于满足刁钻触发条件时调用）
    bool triggerHiddenQuest(int index);

    // ---- 按 questId 查找 ----
    int findTaskByQuestId(const std::string& questId) const;

    // ---- 获取某类型任务数量 ----
    int countByType(QuestType type) const;

    // ---- 状态查询 ----
    TaskStatus getTaskStatusByIndex(int index) const;

    // ---- 事件消息 ----
    int broadcastMessage(const TaskMessage& msg);
    void checkAllAutoComplete();

    // ---- 详情 ----
    bool showTaskDetail(int index) const;
    int size() const;

    // 旧版 showTasks（无参数）
    void showTasks() const { showTasksOverview(); }
};

#endif
