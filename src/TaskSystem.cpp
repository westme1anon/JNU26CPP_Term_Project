// TaskSystem.cpp
#include "TaskSystem.h"
#include "ConsoleUI.h"
#include "GameConfig.h"

#include <iostream>
#include <fstream>
#include <sstream>

TaskSystem::TaskSystem()
{
    loadTasks();
}

void TaskSystem::loadTasks()
{
    tasks.clear();

    std::ifstream file(GameConfig::TASKS_FILE_PATH);
    if (!file.is_open())
    {
        std::cerr << "警告: 无法打开任务数据文件 " << GameConfig::TASKS_FILE_PATH << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::stringstream ss(line);
        std::string name, description, condition, rewardExpStr, rewardGoldStr;

        std::getline(ss, name, '|');
        std::getline(ss, description, '|');
        std::getline(ss, condition, '|');
        std::getline(ss, rewardExpStr, '|');
        std::getline(ss, rewardGoldStr, '|');

        int rewardExp = std::stoi(rewardExpStr);
        int rewardGold = std::stoi(rewardGoldStr);

        tasks.emplace_back(name, description, condition, rewardExp, rewardGold);
    }
}

void TaskSystem::showTasks() const
{
    if (tasks.empty())
    {
        std::cout << "当前没有可用任务。\n";
        return;
    }

    ConsoleUI::printLine('-', 60);

    std::cout << "编号  任务名称            状态\n";
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
