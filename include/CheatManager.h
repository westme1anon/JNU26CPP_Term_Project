// CheatManager.h
#ifndef CHEAT_MANAGER_H
#define CHEAT_MANAGER_H

#include "Character.h"

#include <string>
#include <vector>
#include <functional>

// ============================================================
// CheatEntry
// ------------------------------------------------------------
// 单条作弊码配置结构体。
// 从 cheats.json 解析填充。
// ============================================================

struct CheatEntry
{
    std::string code;         // 作弊指令名，如 "add_gold"
    std::string type;         // 执行类型：modify_int / set_int / toggle_bool / call_func
    std::string target;       // 目标路径，如 "player.gold"
    std::string param;        // 参数名，如 "amount"
    std::string description;  // 中文说明
};

// ============================================================
// CheatManager
// ------------------------------------------------------------
// 作弊码管理器。
// 游戏启动时从 cheats.json 读取配置，根据配置自动注册指令。
// 提供 ExecuteCommand 接口解析并执行用户输入的作弊指令。
// ============================================================

class CheatManager
{
private:
    std::vector<CheatEntry> cheats;

public:
    CheatManager();

    // 从 JSON 文件加载作弊码配置。
    void loadCheats(const std::string& filepath);

    // 解析并执行一条作弊指令。
    // 格式：<code> [参数值]
    // 返回 true 表示成功执行。
    bool executeCommand(const std::string& input, Character& player, bool& shopUnlocked);

    // 列出所有可用作弊码。
    void listCheats() const;

private:
    // 按指令名查找作弊码配置。
    const CheatEntry* findCheat(const std::string& code) const;

    // 执行不同类型的作弊指令。
    bool executeModifyInt(const CheatEntry& entry, Character& player, int value);
    bool executeSetInt(const CheatEntry& entry, Character& player, int value);
    bool executeToggleBool(const CheatEntry& entry, Character& player, bool& shopUnlocked);
    bool executeCallFunc(const CheatEntry& entry, Character& player, bool& shopUnlocked);
};

#endif
