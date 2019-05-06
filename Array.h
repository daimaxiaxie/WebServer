//
// Created by user on 19-4-9.
//

#ifndef WEBSERVER_ARRAY_H
#define WEBSERVER_ARRAY_H

#include "MemoryPool.h"

struct Array {
    void *addr;
    uintptr_t used;
    size_t size;
    uintptr_t capacity;
    PoolHeader *pool;
    //BlockHeader *block;
};

Array *CreateArray(PoolHeader *pool, uintptr_t n, size_t size);

bool InitArray(Array *array, PoolHeader *pool, uintptr_t n, size_t size);

void *ArrayPush(Array *array);

void *ArrayPush_N(Array *array, uintptr_t n);

void ClearArray(Array *array);

void DestroyArray(Array *array);


#endif //WEBSERVER_ARRAY_H
