//
// Created by user on 19-3-30.
//

#include "Lock.h"


Lock::Lock(void *shmaddr) {
    counter = (std::atomic_int *) shmaddr;
    //std::atomic_store(counter,x);
    old = 1;
    islock = false;
}

Lock::~Lock() {

}

void Lock::setCounter(int x) {
    std::atomic_store(counter, x);
}

bool Lock::islocked() {
    return islock;
}

bool Lock::trylock() {
    //__sync_bool_compare_and_swap(*counter,1,0);
    //std::atomic_int old(1);
    old = 1;
    //int x=0;
    if (std::atomic_load(counter) > 0 && std::atomic_compare_exchange_weak(counter, &old, 0)) {
        islock = true;
        return true;
    }
    return false;
}

bool Lock::unlock() {
    old = 0;
    if (islock && std::atomic_compare_exchange_weak(counter, &old, 1)) {
        islock = false;
        return true;
    }
    return false;
}