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
// 新增：事件消息监测 - 收到匹配事件时自动更新激活任务的进度。
// ============================================================

class TaskSystem
{
private:
    std::vector<Task> tasks;

    // 旧格式兼容加载（管道分隔文本）
    void loadLegacyTasks();

public:
    friend class SaveManager;

    TaskSystem();

    // 加载任务数据（JSON 格式）
    void loadTasks();

    // 显示所有任务
    void showTasks() const;

    // 显示指定任务详细信息
    bool showTaskDetail(int index) const;

    // 接受指定任务
    bool acceptTask(int index);

    // 手动完成指定任务
    bool completeTask(int index);

    // 领取指定任务奖励
    bool claimReward(int index, Character& player);

    // ---- 新增：事件消息处理 ----

    // 广播一条任务消息给所有已接受的任务
    // 自动匹配子目标、推进进度，并在所有目标完成时自动切换状态
    // 返回被更新的任务数量
    int broadcastMessage(const TaskMessage& msg);

    // 为指定玩家检查所有进行中的任务是否可自动完成
    // 通常在任务进度更新后调用
    void checkAllAutoComplete();

    // 获取任务数量
    int size() const;
};

#endif
