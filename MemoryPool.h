//
// Created by user on 19-4-9.
//

#ifndef WEBSERVER_MEMORYPOOL_H
#define WEBSERVER_MEMORYPOOL_H

#include <memory>
#include <string.h>


#ifdef __x86_64__
#define ALIGNMENT 0x40
/*#elif __i386__
#define ALIGNMENT 0x20*/
#else
#define ALIGNMENT 0x20
#endif // __x86_64__


struct PoolLarge {
    PoolLarge *next;
    void *data;
};

struct BlockHeader {
    unsigned char *last;
    unsigned char *end;
    BlockHeader *next;
    uintptr_t failed;
};

struct PoolHeader {
    BlockHeader header;
    size_t max;
    BlockHeader *current;
    PoolLarge *large;
};

PoolHeader *CreateMemoryPool(size_t size);

void DestroyMemoryPool(PoolHeader *pool);

void *palloc(size_t size, bool zero);

void *pmalloc(PoolHeader *pool, size_t size, bool Alignment);

void *pmalloc_large(PoolHeader *pool, size_t size);

void *pmalloc_block(PoolHeader *pool, size_t size);

void pfree(PoolHeader *pool, void *p);

void ResetMemoryPool(PoolHeader *pool);


#endif //WEBSERVER_MEMORYPOOL_H
