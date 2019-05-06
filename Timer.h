//
// Created by user on 19-4-19.
//

#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include <chrono>
#include <unistd.h>

union TimeID {
    struct {
        int time;
        int id;
    } s;
    long long timeID;

    long long &operator()(int time, int &id) {
        s.time = time;
        s.id = id;
        return timeID;
    }
};

class Timer {
public:
    Timer() = default;

    ~Timer() = default;

    void Start();

    void End();

    long Delta();

    void TimeUpdate();

    std::chrono::milliseconds GetTime(std::chrono::steady_clock::time_point time);

    void InitID();

    long long makeID();


private:
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;
    std::chrono::milliseconds duration;

    std::chrono::steady_clock::time_point baseTime;
    std::chrono::steady_clock::time_point nowTime;
    int serial;
};


#endif //WEBSERVER_TIMER_H
