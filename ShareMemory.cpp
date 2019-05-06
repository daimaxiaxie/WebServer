//
// Created by user on 19-3-29.
//

#include "ShareMemory.h"

int CreateShareMemory() {

    return shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0644);
}

void *BindShareMemory(int shmid) {
    return shmat(shmid, 0, 0);
}

int DetachShareMemory(void *shmaddr) {
    return shmdt(shmaddr);
}

int DestroyShareMemory(int shmid) {
    return shmctl(shmid, IPC_RMID, 0);
}
