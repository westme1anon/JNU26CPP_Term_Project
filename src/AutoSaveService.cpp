// AutoSaveService.cpp
#include "AutoSaveService.h"

#include "GameConfig.h"
#include "GameManager.h"

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
    if (running)
    {
        return;
    }

    running = true;
    worker = std::thread(&AutoSaveService::autoSaveLoop, this, manager);
}

void AutoSaveService::stop()
{
    running = false;
    wakeSignal.notify_all();

    if (worker.joinable())
    {
        worker.join();
    }
}

void AutoSaveService::autoSaveLoop(GameManager* manager)
{
    while (running)
    {
        std::unique_lock<std::mutex> lock(wakeMutex);
        wakeSignal.wait_for(
            lock,
            std::chrono::seconds(intervalSeconds),
            [this]() { return !running.load(); }
        );

        if (!running)
        {
            break;
        }

        lock.unlock();

        if (manager)
        {
            manager->saveGame();
        }
    }
}
