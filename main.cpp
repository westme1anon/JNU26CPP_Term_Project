// main.cpp

#include "include/EnvLoader.h"
#include "include/GameManager.h"

#include <iostream>

// ============================================================
// main.cpp
// ------------------------------------------------------------
// 程序入口。
// 创建 GameManager 对象并启动游戏。
// ============================================================

int main()
{
    loadEnvFile(".env");

    GameManager game;

    game.init();
    game.run();

    return 0;
}
