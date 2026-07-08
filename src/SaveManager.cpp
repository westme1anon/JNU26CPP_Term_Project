// SaveManager.cpp
#include "SaveManager.h"
#include "GameConfig.h"
#include "Item.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

SaveManager::SaveManager()
    : savePath(GameConfig::SAVE_FILE_PATH)
{
}

SaveManager::SaveManager(const std::string& customPath)
    : savePath(customPath)
{
}

bool SaveManager::saveGame(
    const Character& player,
    const Inventory& inventory,
    const TaskSystem& taskSystem
)
{
    // 确保存档目录存在
    std::filesystem::path filePath(savePath);
    std::filesystem::path dir = filePath.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }
    std::ofstream ofs(savePath);
    if (!ofs.is_open())
    {
        std::cerr << "无法打开存档文件进行保存: " << savePath << std::endl;
        return false;
    }
    // 1. 保存角色数据
    ofs << "[Character]\n";
    std::string charData = player.serialize();
    ofs << charData;
    if (!charData.empty() && charData.back() != '\n')
    {
        ofs << "\n";
    }
    // 2. 保存背包物品数据
    ofs << "[Inventory]\n";
    for (int i = 0; i < inventory.size(); ++i)
    {
        const Item* item = inventory.getItem(i);
        if (!item) continue;
        std::string type = item->getType();
        std::string name = item->getName();
        std::string desc = item->getDescription();
        int price = item->getPrice();
        if (type == "Food")
        {
            // 访问FoodItem的recoverHp私有字段（需在FoodItem中将SaveManager声明为友元）
            const FoodItem* food = dynamic_cast<const FoodItem*>(item);
            int recover = food ? food->recoverHp : 0;
            ofs << "Food|" << name << "|" << desc << "|" << price << "|" << recover << "|0\n";
        }
        else if (type == "Medicine")
        {
            const MedicineItem* med = dynamic_cast<const MedicineItem*>(item);
            int heal = med ? med->healAmount : 0;
            ofs << "Medicine|" << name << "|" << desc << "|" << price << "|" << heal << "|0\n";
        }
        else if (type == "Equipment")
        {
            const EquipmentItem* equip = dynamic_cast<const EquipmentItem*>(item);
            int atk = equip ? equip->attackBonus : 0;
            int def = equip ? equip->defenseBonus : 0;
            ofs << "Equipment|" << name << "|" << desc << "|" << price << "|" << atk << "|" << def << "\n";
        }
    }
    // 3. 保存任务状态数据
    ofs << "[Tasks]\n";
    // 需在TaskSystem中将SaveManager声明为友元，以便访问任务列表
    for (const Task& task : taskSystem.tasks)
    {
        TaskStatus st = task.getStatus();
        std::string statusText;
        switch (st)
        {
        case TaskStatus::NotAccepted:    statusText = "NotAccepted";  break;
        case TaskStatus::Accepted:       statusText = "Accepted";     break;
        case TaskStatus::Completed:      statusText = "Completed";    break;
        case TaskStatus::RewardClaimed:  statusText = "RewardClaimed"; break;
        }
        ofs << task.getName() << "|" << statusText << "\n";
    }
    ofs.close();
    return true;
}

bool SaveManager::loadGame(
    Character& player,
    Inventory& inventory,
    TaskSystem& taskSystem
)
{
    // 1. 检查存档文件是否存在
    std::ifstream ifs(savePath);
    if (!ifs.is_open())
    {
        return false;
    }
    std::string line;
    bool readingChar = false;
    bool readingInv = false;
    bool readingTasks = false;
    std::string charData;
    while (std::getline(ifs, line))
    {
        if (line == "[Character]")
        {
            readingChar = true;
            continue;
        }
        if (line == "[Inventory]")
        {
            // 角色部分读取完毕，反序列化角色数据
            if (!charData.empty())
            {
                player.deserialize(charData);
                charData.clear();
            }
            readingChar = false;
            readingInv = true;
            continue;
        }
        if (line == "[Tasks]")
        {
            // 背包部分读取完毕
            readingInv = false;
            readingTasks = true;
            continue;
        }
        if (readingChar)
        {
            // 收集角色数据的各属性行
            charData += line + "\n";
        }
        else if (readingInv)
        {
            if (line.empty()) continue;
            // 解析背包物品
            std::stringstream iss(line);
            std::string type, name, desc;
            std::string priceStr, v1Str, v2Str;
            std::getline(iss, type, '|');
            std::getline(iss, name, '|');
            std::getline(iss, desc, '|');
            std::getline(iss, priceStr, '|');
            std::getline(iss, v1Str, '|');
            std::getline(iss, v2Str);
            int price = std::stoi(priceStr);
            int val1 = std::stoi(v1Str);
            int val2 = std::stoi(v2Str);
            if (type == "Food")
            {
                inventory.addItem(std::make_unique<FoodItem>(name, desc, price, val1));
            }
            else if (type == "Medicine")
            {
                inventory.addItem(std::make_unique<MedicineItem>(name, desc, price, val1));
            }
            else if (type == "Equipment")
            {
                inventory.addItem(std::make_unique<EquipmentItem>(name, desc, price, val1, val2));
            }
        }
        else if (readingTasks)
        {
            if (line.empty()) continue;
            // 如有需要，确保任务列表已加载
            if (taskSystem.size() == 0)
            {
                taskSystem.loadTasks();
            }
            // 解析任务状态行
            size_t pos = line.find('|');
            if (pos == std::string::npos) continue;
            std::string taskName = line.substr(0, pos);
            std::string statusStr = line.substr(pos + 1);
            TaskStatus status = TaskStatus::NotAccepted;
            if (statusStr == "NotAccepted")      status = TaskStatus::NotAccepted;
            else if (statusStr == "Accepted")    status = TaskStatus::Accepted;
            else if (statusStr == "Completed")   status = TaskStatus::Completed;
            else if (statusStr == "RewardClaimed") status = TaskStatus::RewardClaimed;
            else continue;
            // 查找对应任务并更新状态
            for (size_t i = 0; i < taskSystem.tasks.size(); ++i)
            {
                Task& t = taskSystem.tasks[i];
                if (t.getName() == taskName)
                {
                    switch (status)
                    {
                    case TaskStatus::NotAccepted:
                        // 初始状态，无需改变
                        break;
                    case TaskStatus::Accepted:
                        t.accept();
                        break;
                    case TaskStatus::Completed:
                        t.accept();
                        t.complete();
                        break;
                    case TaskStatus::RewardClaimed:
                    {
                        t.accept();
                        t.complete();
                        // 使用临时角色对象来标记已领取奖励（避免重复增加奖励）
                        Character dummy;
                        dummy.create("dummy");
                        t.claimReward(dummy);
                        break;
                    }
                    }
                    break;
                }
            }
        }
    }
    // 文件读取结束后检查：若未检测到任务段，则加载任务列表
    if (!readingTasks)
    {
        taskSystem.loadTasks();
    }
    // 若角色属性未反序列化（文件无背包段时）
    if (!charData.empty())
    {
        player.deserialize(charData);
    }
    ifs.close();
    return true;
}

bool SaveManager::hasSaveFile() const
{
    std::ifstream file(savePath);
    return file.good();
}
