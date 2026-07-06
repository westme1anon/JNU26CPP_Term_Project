// AutoSaveService.cpp
#include "AutoSaveService.h"
#include "GameManager.h"
#include "GameConfig.h"

#include <chrono>
#include <iostream>

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
    // TODO:
    // 1. 如果已经运行则直接返回
    // 2. 设置 running = true
    // 3. 创建后台线程执行 autoSaveLoop
}

void AutoSaveService::stop()
{
    // TODO:
    // 1. 设置 running = false
    // 2. 如果 worker 可 join，则 join
}

void AutoSaveService::autoSaveLoop(GameManager* manager)
{
    // TODO:
    // while running:
    //     sleep intervalSeconds
    //     调用 manager->saveGame()
}