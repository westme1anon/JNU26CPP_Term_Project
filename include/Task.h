// Task.h
#ifndef TASK_H_TASK_H
#define TASK_H_TASK_H

#include "Character.h"

#include <string>
#include <vector>
#include <iostream>

// ============================================================
// 任务状态枚举。
// ============================================================

enum class TaskStatus {
    NotAccepted,
    Accepted,
    Completed,
    RewardClaimed
};

// ============================================================
// 任务类型枚举。
// ============================================================

enum class QuestType {
    Main,
    World,
    Hidden
};

inline const char* questTypeToString(QuestType t) {
    switch (t) {
    case QuestType::Main:   return "主线";
    case QuestType::World:  return "世界";
    case QuestType::Hidden: return "隐藏";
    }
    return "未知";
}

// ============================================================
// TaskMessage / Objective
// ------------------------------------------------------------
// 任务事件消息，用于驱动任务进度自动更新。
// type: 事件类型 (kill / collect / gold / battle_win 等)
// target: 事件目标名称
// value: 事件数值
// ============================================================

struct TaskMessage {
    std::string type;
    std::string target;
    int value;
};

struct Objective {
    std::string type;
    std::string target;
    int required;
    int current;
    std::string description;
};

// ============================================================
// Task 类
// ------------------------------------------------------------
// 包含任务名称、描述、多子目标、奖励和当前状态。
// 支持按类型分类（主线/世界/隐藏）、前置条件、默认接取、
// 可见性控制等。
// ============================================================

class Task {
    friend class SaveManager;

private:
    std::string name;
    std::string description;
    std::vector<Objective> objectives;
    int rewardExp = 0;
    int rewardGold = 0;
    TaskStatus status = TaskStatus::NotAccepted;

    // ---- 新维度字段 ----
    QuestType questType = QuestType::World;
    std::string questId;                // e.g. "q_main001"
    std::string prerequisiteQuestId;    // 前置任务ID，空即无前置
    int minLevel = 1;                   // 最低接取等级
    bool defaultAccepted = false;       // 创建角色后自动接取
    bool visible = true;                // false 时不显示（隐藏任务）
    bool onCompleteUnlockShop = false;  // 完成后解锁商店

public:
    Task();
    Task(const std::string& name,
         const std::string& description,
         const std::vector<Objective>& objectives,
         int rewardExp,
         int rewardGold,
         QuestType questType = QuestType::World,
         const std::string& questId = "",
         const std::string& prerequisiteQuestId = ""
    );

    void showInfo() const;
    void accept();
    void complete();
    void claimReward(Character& player);

    bool onMessage(const TaskMessage& msg);
    bool isAllObjectivesComplete() const;
    bool checkAndAutoComplete();

    const std::vector<Objective>& getObjectives() const { return objectives; }
    void resetProgress();

    // ---- 访问器 ----
    TaskStatus getStatus() const;
    QuestType getQuestType() const { return questType; }
    const std::string& getQuestId() const { return questId; }
    const std::string& getPrerequisiteQuestId() const { return prerequisiteQuestId; }
    int getMinLevel() const { return minLevel; }
    bool isDefaultAccepted() const { return defaultAccepted; }
    bool isVisible() const { return visible; }
    bool getOnCompleteUnlockShop() const { return onCompleteUnlockShop; }

    void setQuestType(QuestType t) { questType = t; }
    void setQuestId(const std::string& id) { questId = id; }
    void setPrerequisiteQuestId(const std::string& id) { prerequisiteQuestId = id; }
    void setMinLevel(int lv) { minLevel = lv; }
    void setDefaultAccepted(bool v) { defaultAccepted = v; }
    void setVisible(bool v) { visible = v; }
    void setOnCompleteUnlockShop(bool v) { onCompleteUnlockShop = v; }

    std::string getName() const;
};

#endif
