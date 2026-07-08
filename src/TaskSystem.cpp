// TaskSystem.cpp
#include "TaskSystem.h"
#include "ConsoleUI.h"
#include "GameConfig.h"
#include "SimpleJson.h"
#include "PathUtil.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// ---- 辅助：解析 questType 字符串 ----
static QuestType parseQuestType(const std::string& s)
{
    if (s == "main" || s == "Main")   return QuestType::Main;
    if (s == "world" || s == "World") return QuestType::World;
    if (s == "hidden" || s == "Hidden") return QuestType::Hidden;
    return QuestType::World;
}

TaskSystem::TaskSystem()
{
    loadTasks();
}

// ============================================================
// JSON 格式任务加载（新 schema）
// ============================================================

void TaskSystem::loadTasks()
{
    tasks.clear();

    std::cout << "[TaskSystem] 正在加载任务数据...\n";

    try
    {
        JsonValue root = parseJsonFile(GameConfig::TASKS_FILE_PATH);

        if (!root.has("tasks"))
        {
            std::cout << "[TaskSystem] 警告: JSON 文件中未找到 \"tasks\" 数组。\n";
            return;
        }

        const JsonValue& taskArray = root["tasks"];
        if (taskArray.type != JsonValue::Array)
        {
            std::cout << "[TaskSystem] 警告: \"tasks\" 不是数组。\n";
            return;
        }

        for (size_t i = 0; i < taskArray.size(); ++i)
        {
            const JsonValue& t = taskArray[i];

            // ---- 基础字段 ----
            std::string name        = t.has("name")        ? t["name"].asString()        : "未知任务";
            std::string description = t.has("description") ? t["description"].asString() : "暂无描述";
            int rewardExp           = t.has("rewardExp")    ? t["rewardExp"].asInt()      : 0;
            int rewardGold          = t.has("rewardGold")   ? t["rewardGold"].asInt()     : 0;

            // ---- 新增字段 ----
            QuestType questType = QuestType::World;
            if (t.has("questType"))
                questType = parseQuestType(t["questType"].asString());

            std::string questId = t.has("questId") ? t["questId"].asString() : "";
            std::string prerequisiteQuestId = t.has("prerequisiteQuestId") ? t["prerequisiteQuestId"].asString() : "";
            int minLevel = t.has("minLevel") ? t["minLevel"].asInt() : 1;
            bool defaultAccepted = t.has("defaultAccepted") ? (t["defaultAccepted"].asInt() != 0) : false;
            bool visible = t.has("visible") ? (t["visible"].asInt() != 0) : true;
            bool onCompleteUnlockShop = t.has("onCompleteUnlockShop") ? (t["onCompleteUnlockShop"].asInt() != 0) : false;

            // ---- 目标列表 ----
            std::vector<Objective> objectives;
            if (t.has("objectives"))
            {
                const JsonValue& objArray = t["objectives"];
                if (objArray.type == JsonValue::Array)
                {
                    for (size_t j = 0; j < objArray.size(); ++j)
                    {
                        const JsonValue& o = objArray[j];
                        Objective obj;
                        obj.type        = o.has("type")        ? o["type"].asString()        : "";
                        obj.target      = o.has("target")      ? o["target"].asString()      : "";
                        obj.required    = o.has("required")    ? o["required"].asInt()       : 1;
                        obj.current     = 0;
                        obj.description = o.has("description") ? o["description"].asString() : "";
                        objectives.push_back(obj);
                    }
                }
            }

            // 构造任务
            Task task(name, description, objectives, rewardExp, rewardGold,
                      questType, questId, prerequisiteQuestId);
            task.setMinLevel(minLevel);
            task.setDefaultAccepted(defaultAccepted);
            task.setVisible(visible);
            task.setOnCompleteUnlockShop(onCompleteUnlockShop);
            tasks.push_back(task);
        }

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[TaskSystem] 成功加载 " << tasks.size() << " 个任务（JSON 格式）。\n";
        int mainCount = countByType(QuestType::Main);
        int worldCount = countByType(QuestType::World);
        int hiddenCount = countByType(QuestType::Hidden);
        std::cout << "  - 主线任务: " << mainCount << "  世界任务: " << worldCount << "  隐藏任务: " << hiddenCount << "\n";
        ConsoleUI::resetColor();
    }
    catch (const std::exception& e)
    {
        std::cout << "[TaskSystem] JSON 解析错误: " << e.what() << "\n";
        std::cout << "[TaskSystem] 尝试回退到旧格式 tasks.txt ...\n";
        loadLegacyTasks();
    }
}

// ============================================================
// 旧格式兼容加载
// ============================================================

void TaskSystem::loadLegacyTasks()
{
    tasks.clear();

    std::string resolved = PathUtil::resolvePath("data/tasks.txt");
    if (resolved.empty()) resolved = "data/tasks.txt";

    std::cout << "[TaskSystem] 尝试加载旧格式: " << resolved << "\n";

    std::ifstream file(resolved);
    if (!file.is_open())
    {
        std::cout << "[TaskSystem] 警告: 无法打开旧格式任务文件 " << resolved << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string name, description, condition, rewardExpStr, rewardGoldStr;

        std::getline(ss, name, '|');
        std::getline(ss, description, '|');
        std::getline(ss, condition, '|');
        std::getline(ss, rewardExpStr, '|');
        std::getline(ss, rewardGoldStr);

        int rewardExp = std::stoi(rewardExpStr);
        int rewardGold = std::stoi(rewardGoldStr);

        std::vector<Objective> objectives;
        Objective obj;
        obj.type = "manual";
        obj.target = "";
        obj.required = 1;
        obj.current = 0;
        obj.description = condition;
        objectives.push_back(obj);

        tasks.emplace_back(name, description, objectives, rewardExp, rewardGold);
    }

    std::cout << "[TaskSystem] 旧格式加载了 " << tasks.size() << " 个任务。\n";
}

// ============================================================
// 自动接取默认任务
// ============================================================

void TaskSystem::autoAcceptDefaults()
{
    for (auto& task : tasks)
    {
        if (task.isDefaultAccepted() && task.getStatus() == TaskStatus::NotAccepted)
        {
            task.accept();
        }
    }
}

// ============================================================
// 分类统计
// ============================================================

int TaskSystem::countByType(QuestType type) const
{
    int count = 0;
    for (const auto& t : tasks)
        if (t.getQuestType() == type)
            ++count;
    return count;
}

// ============================================================
// 按 questId 查找
// ============================================================

int TaskSystem::findTaskByQuestId(const std::string& questId) const
{
    for (size_t i = 0; i < tasks.size(); ++i)
        if (tasks[i].getQuestId() == questId)
            return static_cast<int>(i);
    return -1;
}

TaskStatus TaskSystem::getTaskStatusByIndex(int index) const
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
        return TaskStatus::NotAccepted;
    return tasks[index].getStatus();
}

// ============================================================
// 条件校验
// ============================================================

bool TaskSystem::canAcceptTask(int index, const Character& player) const
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
        return false;

    const Task& task = tasks[index];

    // 状态检查
    if (task.getStatus() != TaskStatus::NotAccepted)
        return false;

    // 等级检查
    if (player.getLevel() < task.getMinLevel())
        return false;

    // 前置任务检查
    const std::string& prereq = task.getPrerequisiteQuestId();
    if (!prereq.empty())
    {
        int prereqIndex = findTaskByQuestId(prereq);
        if (prereqIndex < 0)
            return false;
        TaskStatus prereqStatus = tasks[prereqIndex].getStatus();
        if (prereqStatus != TaskStatus::Completed && prereqStatus != TaskStatus::RewardClaimed)
            return false;
    }

    return true;
}

// ============================================================
// 接受任务（带校验）
// ============================================================

bool TaskSystem::acceptTask(int index, const Character& player)
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的任务编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    Task& task = tasks[index];

    // 状态检查
    if (task.getStatus() != TaskStatus::NotAccepted)
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "该任务当前状态不允许接取。\n";
        ConsoleUI::resetColor();
        return false;
    }

    // 等级检查
    if (player.getLevel() < task.getMinLevel())
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "你的等级不足！（需要 " << task.getMinLevel() << " 级，当前 " << player.getLevel() << " 级）\n";
        ConsoleUI::resetColor();
        return false;
    }

    // 前置任务检查
    const std::string& prereq = task.getPrerequisiteQuestId();
    if (!prereq.empty())
    {
        int prereqIndex = findTaskByQuestId(prereq);
        if (prereqIndex < 0)
        {
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "前置任务 " << prereq << " 不存在！\n";
            ConsoleUI::resetColor();
            return false;
        }
        TaskStatus prereqStatus = tasks[prereqIndex].getStatus();
        if (prereqStatus != TaskStatus::Completed && prereqStatus != TaskStatus::RewardClaimed)
        {
            ConsoleUI::setColor(GameConfig::COLOR_ERROR);
            std::cout << "请先完成任务: " << tasks[prereqIndex].getName() << " ！\n";
            ConsoleUI::resetColor();
            return false;
        }
    }

    task.accept();
    return true;
}

// ============================================================
// 隐藏任务触发
// ============================================================

bool TaskSystem::triggerHiddenQuest(int index)
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
        return false;

    Task& task = tasks[index];

    if (task.getQuestType() != QuestType::Hidden)
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "该任务不是隐藏任务。\n";
        ConsoleUI::resetColor();
        return false;
    }

    if (task.isVisible())
    {
        ConsoleUI::setColor(GameConfig::COLOR_WARNING);
        std::cout << "该隐藏任务已经被触发过了。\n";
        ConsoleUI::resetColor();
        return false;
    }

    task.setVisible(true);
    task.accept();

    ConsoleUI::setColor(GameConfig::COLOR_TITLE);
    std::cout << "\n============================================================\n";
    std::cout << "[!] 似乎有什么不对劲……\n";
    std::cout << "[隐藏任务触发] " << task.getName() << "\n";
    std::cout << "============================================================\n\n";
    ConsoleUI::resetColor();

    return true;
}

// ============================================================
// 事件消息广播
// ============================================================

int TaskSystem::broadcastMessage(const TaskMessage& msg)
{
    int updatedCount = 0;

    for (auto& task : tasks)
    {
        if (task.onMessage(msg))
        {
            ++updatedCount;
        }
    }

    if (updatedCount > 0)
    {
        ConsoleUI::setColor(GameConfig::COLOR_TITLE);
        std::cout << "[TaskSystem] 事件 [" << msg.type;
        if (!msg.target.empty())
            std::cout << ":" << msg.target;
        std::cout << " x" << msg.value << "] 触发了 " << updatedCount << " 个任务进度更新。\n";
        ConsoleUI::resetColor();
    }

    return updatedCount;
}

void TaskSystem::checkAllAutoComplete()
{
    for (auto& task : tasks)
    {
        task.checkAndAutoComplete();
    }
}

// ============================================================
// 分类总览
// ============================================================

void TaskSystem::showTasksOverview() const
{
    ConsoleUI::printLine('-', 60);
    std::cout << "任务总览 - 共 " << tasks.size() << " 个任务\n";
    ConsoleUI::printLine('-', 60);

    for (int typeVal = 0; typeVal < 3; ++typeVal)
    {
        QuestType qt = static_cast<QuestType>(typeVal);
        std::string typeName = questTypeToString(qt);
        int color = (qt == QuestType::Main) ? GameConfig::COLOR_TITLE :
                    (qt == QuestType::World) ? GameConfig::COLOR_SUCCESS :
                                               GameConfig::COLOR_WARNING;

        ConsoleUI::setColor(color);
        std::cout << "\n  [" << typeName << "任务]\n";
        ConsoleUI::resetColor();

        bool found = false;
        for (size_t i = 0; i < tasks.size(); ++i)
        {
            const Task& t = tasks[i];
            if (t.getQuestType() != qt) continue;
            if (!t.isVisible()) continue; // 隐藏任务未触发时不显示

            found = true;
            std::cout << "  " << (i + 1) << ". " << t.getName();

            // 状态
            switch (t.getStatus()) {
            case TaskStatus::NotAccepted:
                std::cout << " [未接受]";
                break;
            case TaskStatus::Accepted:
                ConsoleUI::setColor(GameConfig::COLOR_WARNING);
                std::cout << " [进行中]";
                ConsoleUI::resetColor();
                break;
            case TaskStatus::Completed:
                ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
                std::cout << " [可领奖]";
                ConsoleUI::resetColor();
                break;
            case TaskStatus::RewardClaimed:
                ConsoleUI::setColor(GameConfig::COLOR_TITLE);
                std::cout << " [已完成]";
                ConsoleUI::resetColor();
                break;
            }
            std::cout << "\n";
        }
        if (!found)
            std::cout << "  （无）\n";
    }

    ConsoleUI::printLine('-', 60);
}

void TaskSystem::showTasksByType(QuestType type, bool showHidden) const
{
    ConsoleUI::printLine('-', 60);
    int color = (type == QuestType::Main) ? GameConfig::COLOR_TITLE :
                (type == QuestType::World) ? GameConfig::COLOR_SUCCESS :
                                             GameConfig::COLOR_WARNING;
    ConsoleUI::setColor(color);
    std::cout << questTypeToString(type) << "任务列表\n";
    ConsoleUI::resetColor();
    ConsoleUI::printLine('-', 60);

    bool found = false;
    for (size_t i = 0; i < tasks.size(); ++i)
    {
        const Task& t = tasks[i];
        if (t.getQuestType() != type) continue;
        if (!showHidden && !t.isVisible()) continue;

        found = true;
        std::cout << (i + 1) << ". " << t.getName();

        // 显示前置依赖
        if (!t.getPrerequisiteQuestId().empty())
            std::cout << "  <- 需: " << t.getPrerequisiteQuestId();

        switch (t.getStatus()) {
        case TaskStatus::NotAccepted:
            std::cout << " [未接受]";
            break;
        case TaskStatus::Accepted:
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << " [进行中]";
            ConsoleUI::resetColor();
            // 显示子目标进度
            {
                const auto& objs = t.getObjectives();
                if (!objs.empty()) {
                    int totalReq = 0, totalCur = 0;
                    for (const auto& o : objs) { totalReq += o.required; totalCur += o.current; }
                    std::cout << " [" << totalCur << "/" << totalReq << "]";
                }
            }
            break;
        case TaskStatus::Completed:
            ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
            std::cout << " [可领奖]";
            ConsoleUI::resetColor();
            break;
        case TaskStatus::RewardClaimed:
            ConsoleUI::setColor(GameConfig::COLOR_TITLE);
            std::cout << " [已完成]";
            ConsoleUI::resetColor();
            break;
        }
        std::cout << "\n";
    }

    if (!found)
        std::cout << "  （无）\n";

    ConsoleUI::printLine('-', 60);
}

// ============================================================
// 原有接口（兼容）
// ============================================================

bool TaskSystem::completeTask(int index)
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的任务编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    tasks[index].complete();
    return true;
}

bool TaskSystem::claimReward(int index, Character& player)
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的任务编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    tasks[index].claimReward(player);
    return true;
}

bool TaskSystem::showTaskDetail(int index) const
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的任务编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    tasks[index].showInfo();
    return true;
}

int TaskSystem::size() const
{
    return static_cast<int>(tasks.size());
}
