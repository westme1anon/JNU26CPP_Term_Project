// SaveManager.h
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include "Character.h"
#include "Inventory.h"
#include "TaskSystem.h"

#include <string>

// ============================================================
// SaveManager
// ------------------------------------------------------------
// 存档管理类。
// 负责将玩家、背包、任务状态写入文件，并在游戏启动时读取。
// ============================================================

class SaveManager
{
private:
    std::string savePath;

public:
    SaveManager();

    explicit SaveManager(const std::string& savePath);

    // 保存完整游戏数据。
    bool saveGame(
        const Character& player,
        const Inventory& inventory,
        const TaskSystem& taskSystem
    );

    // 读取完整游戏数据。
    bool loadGame(
        Character& player,
        Inventory& inventory,
        TaskSystem& taskSystem
    );

    // 判断存档文件是否存在。
    bool hasSaveFile() const;
};

#endif