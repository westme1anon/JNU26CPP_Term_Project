// main.cpp

#include "include/EnvLoader.h"
#include "include/GameManager.h"

#include <cstdlib>
#include <iostream>
#include <ctime>

// ============================================================
// main.cpp
// ------------------------------------------------------------
// 程序入口。
// 创建 GameManager 对象并启动游戏。
// ============================================================

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    loadEnvFileWithFallback(".env");

    GameManager game;

    game.init();
    game.run();

    return 0;
}
