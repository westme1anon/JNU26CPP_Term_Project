// SaveManager.cpp
#include "SaveManager.h"
#include "GameConfig.h"

#include <fstream>
#include <iostream>

SaveManager::SaveManager()
    : savePath(GameConfig::SAVE_FILE_PATH)
{
}

SaveManager::SaveManager(const std::string& savePath)
    : savePath(savePath)
{
}

bool SaveManager::saveGame(
    const Character& player,
    const Inventory& inventory,
    const TaskSystem& taskSystem
)
{
    // TODO:
    // 1. 打开 savePath
    // 2. 保存 Character::serialize()
    // 3. 保存背包物品
    // 4. 保存任务状态
    // 5. 返回是否成功
    return false;
}

bool SaveManager::loadGame(
    Character& player,
    Inventory& inventory,
    TaskSystem& taskSystem
)
{
    // TODO:
    // 1. 判断存档文件是否存在
    // 2. 读取角色信息
    // 3. 读取背包信息
    // 4. 读取任务状态
    // 5. 返回是否成功
    return false;
}

bool SaveManager::hasSaveFile() const
{
    // TODO: 使用 ifstream 判断文件是否存在。
    return false;
}