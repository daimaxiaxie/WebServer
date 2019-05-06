//
// Created by user on 19-3-29.
//

#ifndef WEBSERVER_SHAREMEMORY_H
#define WEBSERVER_SHAREMEMORY_H

#include <sys/ipc.h>
#include <sys/shm.h>

int CreateShareMemory();

void *BindShareMemory(int shmid);

int DetachShareMemory(void *shmaddr);

int DestroyShareMemory(int shmid);


#endif //WEBSERVER_SHAREMEMORY_H
