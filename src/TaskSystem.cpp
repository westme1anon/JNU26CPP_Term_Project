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

TaskSystem::TaskSystem()
{
    loadTasks();
}

// ============================================================
// JSON 格式任务加载
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

            std::string name        = t.has("name")        ? t["name"].asString()        : "未知任务";
            std::string description = t.has("description") ? t["description"].asString() : "暂无描述";
            int rewardExp           = t.has("rewardExp")    ? t["rewardExp"].asInt()      : 0;
            int rewardGold          = t.has("rewardGold")   ? t["rewardGold"].asInt()     : 0;

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
                        obj.required     = o.has("required")    ? o["required"].asInt()       : 1;
                        obj.current      = 0;
                        obj.description = o.has("description") ? o["description"].asString() : "";
                        objectives.push_back(obj);
                    }
                }
            }

            tasks.emplace_back(name, description, objectives, rewardExp, rewardGold);
        }

        ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
        std::cout << "[TaskSystem] 成功加载 " << tasks.size() << " 个任务（JSON 格式）。\n";
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
// 旧格式兼容加载（管道分隔）
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
// 原有接口
// ============================================================

void TaskSystem::showTasks() const
{
    if (tasks.empty())
    {
        std::cout << "当前没有可用任务。\n";
        return;
    }

    ConsoleUI::printLine('-', 60);
    std::cout << "编号  任务名称            状态      进度\n";
    ConsoleUI::printLine('-', 60);

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        std::cout << (i + 1) << "     ";
        std::cout.width(20);
        std::cout << std::left << tasks[i].getName();

        switch (tasks[i].getStatus())
        {
        case TaskStatus::NotAccepted:
            std::cout << "未接受";
            break;
        case TaskStatus::Accepted:
            ConsoleUI::setColor(GameConfig::COLOR_WARNING);
            std::cout << "进行中";
            ConsoleUI::resetColor();
            break;
        case TaskStatus::Completed:
            ConsoleUI::setColor(GameConfig::COLOR_SUCCESS);
            std::cout << "已完成";
            ConsoleUI::resetColor();
            break;
        case TaskStatus::RewardClaimed:
            ConsoleUI::setColor(GameConfig::COLOR_TITLE);
            std::cout << "已领奖";
            ConsoleUI::resetColor();
            break;
        }

        const auto& objs = tasks[i].getObjectives();
        if (!objs.empty() && tasks[i].getStatus() == TaskStatus::Accepted)
        {
            int totalRequired = 0;
            int totalCurrent = 0;
            for (const auto& o : objs)
            {
                totalRequired += o.required;
                totalCurrent += o.current;
            }
            std::cout << "    [" << totalCurrent << "/" << totalRequired << "]";
        }

        std::cout << "\n";
    }

    ConsoleUI::printLine('-', 60);
}

bool TaskSystem::acceptTask(int index)
{
    if (index < 0 || index >= static_cast<int>(tasks.size()))
    {
        ConsoleUI::setColor(GameConfig::COLOR_ERROR);
        std::cout << "无效的任务编号。\n";
        ConsoleUI::resetColor();
        return false;
    }

    tasks[index].accept();
    return true;
}

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
