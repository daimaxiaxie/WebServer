//
// Created by user on 19-5-1.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(int n) : exit(false) {
    while (n) {
        activeThread.emplace_back([&]() {
            std::unique_lock<std::mutex> lock(mtx);
            lock.unlock();
            while (!exit) {
                std::function<void()> task;
                conditionVariable.wait(lock, [&]() { return exit || !taskQueue.empty(); });
                if (!taskQueue.empty()) {
                    stlmtx.lock();
                    task = std::move(taskQueue.front());
                    taskQueue.pop_front();
                    stlmtx.unlock();
                    task();
                }
            }
        });
        n--;
    }
}

ThreadPool::~ThreadPool() {
    stlmtx.lock();
    taskQueue.clear();
    stlmtx.unlock();
    exit = true;
    conditionVariable.notify_all();
    for (auto &t:activeThread) {
        t.join();
    }
}