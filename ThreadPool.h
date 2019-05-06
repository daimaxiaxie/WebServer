//
// Created by user on 19-5-1.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <functional>
#include <future>
#include <mutex>
#include <list>
#include <queue>

class ThreadPool {
public:
    ThreadPool(int n);

    ~ThreadPool();


    template<typename T, typename ...Args>
    //Args
    bool commit(T &&t, Args &&...args);  //Args &&...args


private:
    std::list<std::thread> activeThread;
    std::deque<std::function<void()>> taskQueue;    //not thread safe

    std::atomic_bool exit;
    std::mutex stlmtx;
    std::mutex mtx;
    std::condition_variable conditionVariable;
};

template<typename T, typename ...Args>
inline bool ThreadPool::commit(T &&t, Args &&...args) {

    auto task = std::bind(std::forward<T>(t), std::forward<Args>(args)...);
    //std::packaged_task<typename std::result_of<T()>::type> task(t);
    //std::future<typename std::result_of<T()>::type> res=task.get_future();
    stlmtx.lock();
    taskQueue.emplace_back([=]() {
        task();
    });
    stlmtx.unlock();
    conditionVariable.notify_one();
    return true;
}

#endif //WEBSERVER_THREADPOOL_H
