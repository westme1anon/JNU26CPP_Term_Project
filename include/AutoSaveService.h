// AutoSaveService.h
#ifndef AUTO_SAVE_SERVICE_H
#define AUTO_SAVE_SERVICE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

class GameManager;

// ============================================================
// AutoSaveService
// ------------------------------------------------------------
// 自动存档服务。
// 使用 std::thread 在后台定时调用 GameManager 的保存逻辑。
// ============================================================

class AutoSaveService
{
private:
    std::thread worker;
    std::atomic<bool> running;
    std::condition_variable wakeSignal;
    std::mutex wakeMutex;
    int intervalSeconds;

    // 自动存档循环。
    void autoSaveLoop(GameManager* manager);

public:
    AutoSaveService();
    ~AutoSaveService();

    // 启动自动存档线程。
    void start(GameManager* manager);

    // 停止自动存档线程。
    void stop();
};

#endif
