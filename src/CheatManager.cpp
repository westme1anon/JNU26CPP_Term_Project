// CheatManager.cpp
#include "CheatManager.h"
#include "SimpleJson.h"
#include "ConsoleUI.h"

#include <iostream>
#include <sstream>
#include <algorithm>

// ============================================================
// CheatManager 构造与初始化
// ============================================================

CheatManager::CheatManager()
{
}

void CheatManager::loadCheats(const std::string& filepath)
{
    try
    {
        JsonValue root = parseJsonFile(filepath);
        if (root.type != JsonValue::Array)
        {
            std::cerr << "[作弊系统] 警告：cheats.json 根元素应为数组。\n";
            return;
        }

        for (size_t i = 0; i < root.size(); ++i)
        {
            const JsonValue& item = root[i];
            CheatEntry entry;
            entry.code        = item["code"].asString();
            entry.type        = item["type"].asString();
            entry.target      = item["target"].asString();
            entry.param       = item.has("param") ? item["param"].asString() : "";
            entry.description = item.has("description") ? item["description"].asString() : "";
            cheats.push_back(entry);
        }

        std::cout << "[作弊系统] 已加载 " << cheats.size() << " 条作弊码。\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "[作弊系统] 加载 cheats.json 失败：" << e.what() << "\n";
    }
}

// ============================================================
// 指令查找
// ============================================================

const CheatEntry* CheatManager::findCheat(const std::string& code) const
{
    for (const auto& entry : cheats)
    {
        if (entry.code == code)
            return &entry;
    }
    return nullptr;
}

// ============================================================
// 执行作弊指令
// ============================================================

bool CheatManager::executeCommand(const std::string& input, Character& player, bool& shopUnlocked)
{
    // 解析输入：<code> [value]
    std::istringstream iss(input);
    std::string code;
    int value = 0;
    bool hasValue = false;

    if (!(iss >> code))
        return false;

    if (iss >> value)
        hasValue = true;

    const CheatEntry* entry = findCheat(code);
    if (!entry)
    {
        std::cout << "未知作弊码：" << code << "。输入 list_cheats 查看可用作弊码。\n";
        return false;
    }

    if (entry->type == "modify_int")
        return executeModifyInt(*entry, player, hasValue ? value : 0);
    else if (entry->type == "set_int")
        return executeSetInt(*entry, player, hasValue ? value : 0);
    else if (entry->type == "toggle_bool")
        return executeToggleBool(*entry, player, shopUnlocked);
    else if (entry->type == "call_func")
        return executeCallFunc(*entry, player, shopUnlocked);
    else
    {
        std::cout << "未知作弊类型：" << entry->type << "\n";
        return false;
    }
}

// ============================================================
// 列出所有作弊码
// ============================================================

void CheatManager::listCheats() const
{
    ConsoleUI::printTitle("可用作弊码");

    for (const auto& entry : cheats)
    {
        std::cout << "  " << entry.code;
        if (!entry.param.empty())
            std::cout << " <" << entry.param << ">";
        std::cout << "  --  " << entry.description << "\n";
    }
}

// ============================================================
// modify_int：对整数属性进行相对增减
// ============================================================

bool CheatManager::executeModifyInt(const CheatEntry& entry, Character& player, int value)
{
    if (entry.target == "player.gold")
    {
        if (value >= 0)
            player.gainGold(value);
        else
            player.costGold(-value);
        std::cout << "[作弊] 金币变化 " << (value >= 0 ? "+" : "") << value
                  << "，当前：" << player.getGold() << "\n";
        return true;
    }

    if (entry.target == "player.exp")
    {
        if (value >= 0)
            player.gainExp(value);
        else
            std::cout << "[作弊] 经验值不能为负。\n";
        std::cout << "[作弊] 经验变化 " << (value >= 0 ? "+" : "") << value
                  << "，当前：" << player.getExp() << "\n";
        return true;
    }

    if (entry.target == "player.attack")
    {
        player.addAttack(value);
        std::cout << "[作弊] 攻击力 " << (value >= 0 ? "+" : "") << value
                  << "，当前：" << player.getAttack() << "\n";
        return true;
    }

    if (entry.target == "player.defense")
    {
        player.addDefense(value);
        std::cout << "[作弊] 防御力 " << (value >= 0 ? "+" : "") << value
                  << "，当前：" << player.getDefense() << "\n";
        return true;
    }

    std::cout << "[作弊] 未识别的目标：" << entry.target << "\n";
    return false;
}

// ============================================================
// set_int：将整数属性设置为指定值
// ============================================================

bool CheatManager::executeSetInt(const CheatEntry& entry, Character& player, int value)
{
    if (entry.target == "player.level")
    {
        if (value < 1) value = 1;
        player.setLevel(value);
        std::cout << "[作弊] 等级设为 " << value
                  << "，HP=" << player.getHp() << "/" << player.getMaxHp()
                  << " ATK=" << player.getAttack()
                  << " DEF=" << player.getDefense() << "\n";
        return true;
    }

    if (entry.target == "player.hp")
    {
        if (value < 0) value = 0;
        int delta = value - player.getHp();
        if (delta >= 0)
            player.heal(delta);
        else
            player.takeDamage(-delta);
        std::cout << "[作弊] 生命值设为 " << value << "（当前：" << player.getHp() << "）\n";
        return true;
    }

    if (entry.target == "player.max_hp")
    {
        if (value < 1) value = 1;
        player.setMaxHp(value);
        std::cout << "[作弊] 最大生命值设为 " << value << "\n";
        return true;
    }

    std::cout << "[作弊] 未识别的目标：" << entry.target << "\n";
    return false;
}

// ============================================================
// toggle_bool：切换布尔状态
// ============================================================

bool CheatManager::executeToggleBool(const CheatEntry& entry, Character& player, bool& shopUnlocked)
{
    if (entry.target == "player.invincible")
    {
        bool current = player.isInvincible();
        player.setInvincible(!current);
        std::cout << "[作弊] 无敌模式：" << (player.isInvincible() ? "开启" : "关闭") << "\n";
        return true;
    }

    if (entry.target == "game.shop_unlocked")
    {
        shopUnlocked = !shopUnlocked;
        std::cout << "[作弊] 校园商店：" << (shopUnlocked ? "已解锁" : "已锁定") << "\n";
        return true;
    }

    std::cout << "[作弊] 未识别的目标：" << entry.target << "\n";
    return false;
}

// ============================================================
// call_func：调用特定函数
// ============================================================

bool CheatManager::executeCallFunc(const CheatEntry& entry, Character& player, bool& /*shopUnlocked*/)
{
    if (entry.target == "player.restore_hp")
    {
        player.heal(player.getMaxHp());
        std::cout << "[作弊] 生命值已回满（" << player.getHp() << "/" << player.getMaxHp() << "）\n";
        return true;
    }

    if (entry.target == "player.max_stats")
    {
        player.maxStats();
        std::cout << "[作弊] 所有属性已最大化！\n";
        player.showInfo();
        return true;
    }

    if (entry.target == "cheat.list")
    {
        listCheats();
        return true;
    }

    std::cout << "[作弊] 未识别的目标：" << entry.target << "\n";
    return false;
}
