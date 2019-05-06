//
// Created by user on 19-3-30.
//

#ifndef WEBSERVER_LOCK_H
#define WEBSERVER_LOCK_H

#include <atomic>
#include "ShareMemory.h"

class Lock {

public:
    Lock(void *shmaddr);

    ~Lock();

    void setCounter(int x);

    bool islocked();

    bool trylock();

    bool unlock();

private:
    std::atomic_int *counter;
    int old;
    bool islock;

};


#endif //WEBSERVER_LOCK_H
