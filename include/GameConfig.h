#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <string>

// ============================================================
// GameConfig.h
// ------------------------------------------------------------
// 游戏全局配置文件。
// 用于统一管理存档路径、默认数值、控制台颜色等常量。
// ============================================================

namespace GameConfig
{
    // 存档路径
    const std::string SAVE_FILE_PATH = "data/player_save.txt";

    // 数据文件路径
    const std::string SHOP_ITEMS_FILE_PATH = "data/shop_items.txt";
    const std::string TASKS_FILE_PATH = "data/tasks.txt";
    const std::string ENEMIES_FILE_PATH = "data/enemies.txt";

    // 玩家默认属性
    const int DEFAULT_LEVEL = 1;
    const int DEFAULT_HP = 100;
    const int DEFAULT_EXP = 0;
    const int DEFAULT_GOLD = 100;
    const int DEFAULT_ATTACK = 15;
    const int DEFAULT_DEFENSE = 5;

    // 自动存档间隔，单位：秒
    const int AUTO_SAVE_INTERVAL_SECONDS = 60;

    // 控制台颜色，可根据 Windows 控制台颜色表调整
    const int COLOR_DEFAULT = 7;
    const int COLOR_TITLE = 11;
    const int COLOR_SUCCESS = 10;
    const int COLOR_WARNING = 14;
    const int COLOR_ERROR = 12;
}

#endif