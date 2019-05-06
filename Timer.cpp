//
// Created by user on 19-4-19.
//

#include "Timer.h"

void Timer::Start() {

    start = std::chrono::steady_clock::now();

}

void Timer::End() {

    end = std::chrono::steady_clock::now();

}

long Timer::Delta() {
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return long(duration.count());
}

void Timer::TimeUpdate() {
    baseTime = std::chrono::steady_clock::now();
}

std::chrono::milliseconds Timer::GetTime(std::chrono::steady_clock::time_point time) {
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(time - baseTime);

    return duration;
}

void Timer::InitID() {
    TimeUpdate();
    nowTime = std::chrono::steady_clock::now();
    serial = 0;
}

long long Timer::makeID() {

    //64 bits id;
    //0 flag position(positive),45 bits time(millisecond),6 bits machine id,12 bits serial number

    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(time - baseTime);
    long long t = duration.count();
    if (time == nowTime) {
        serial++;
    } else {
        nowTime = time;
        serial = 0;
    }
    if (serial > 4094) {
        t++;
        usleep(100);
        serial -= 4095;
        nowTime = std::chrono::steady_clock::now();
    }
    if (t > 0xFFFFFFFFFF) {
        TimeUpdate();
    }
    long long id = t & 0x01FFFFFFFFFFF;
    id = id << 6;
    id += 1;
    id = id << 12;
    id += serial;

    return id;
}
