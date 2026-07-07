// AutoSaveService.cpp
#include "AutoSaveService.h"
#include "GameManager.h"
#include "GameConfig.h"
#include <chrono>
#include <thread>

AutoSaveService::AutoSaveService()
    : running(false),
      intervalSeconds(GameConfig::AUTO_SAVE_INTERVAL_SECONDS)
{
}

AutoSaveService::~AutoSaveService()
{
    stop();
}

void AutoSaveService::start(GameManager* manager)
{
    // 1. 如果已在运行则直接返回
    if (running)
        return;
    running = true;
    // 2. 创建后台线程执行自动存档循环
    worker = std::thread(&AutoSaveService::autoSaveLoop, this, manager);
}

void AutoSaveService::stop()
{
    // 1. 设置退出标志
    running = false;
    // 2. 等待线程完成
    if (worker.joinable())
    {
        worker.join();
    }
}

void AutoSaveService::autoSaveLoop(GameManager* manager)
{
    // 循环定时保存游戏
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        if (!running) break;
        if (manager)
        {
            manager->saveGame();
        }
    }
}