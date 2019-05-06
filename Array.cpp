//
// Created by user on 19-4-9.
//

#include "Array.h"


Array *CreateArray(PoolHeader *pool, uintptr_t n, size_t size) {
    Array *array;
    array = (Array *) pmalloc(pool, sizeof(Array), true);
    if (array == nullptr) {
        return nullptr;
    }
    if (!InitArray(array, pool, n, size)) {
        return nullptr;
    }
    return array;
}

bool InitArray(Array *array, PoolHeader *pool, uintptr_t n, size_t size) {
    array->used = 0;
    array->size = size;
    array->capacity = n;
    array->pool = pool;

    size_t sizes = n * size;
    array->addr = pmalloc(pool, sizes, true);
    if (array->addr == nullptr) {
        return false;
    }

    /*
    BlockHeader *p = &pool->header;
    while ((void*)array > (void*)p && (unsigned char*)array->addr + sizes <= p->last)
    {
        p = p->next;
    }
    array->block = p;
    */
    return true;
}

void *ArrayPush(Array *array) {
    size_t size;

    if (array->used == array->capacity) {
        size = array->size * array->capacity;
        if (((unsigned char *) array->addr + size) == array->pool->header.last &&
            (array->pool->header.last + array->size) <= array->pool->header.end) {
            array->pool->header.last += array->size;
            array->capacity++;
        } else {
            void *newaddr = pmalloc(array->pool, 2 * size, true);
            if (newaddr == nullptr) {
                return nullptr;
            }

            memcpy(newaddr, array->addr, size);
            //memcpy_s(newaddr, 2 * size, array->addr, size);
            array->addr = newaddr;
            array->capacity *= 2;
        }
    }

    void *addr = (unsigned char *) array->addr + array->size * array->used;
    array->used++;
    return addr;
}

void *ArrayPush_N(Array *array, uintptr_t n) {
    size_t size;
    size = array->size * n;
    if (array->used + n > array->capacity) {
        PoolHeader *p = array->pool;
        if (((unsigned char *) array->addr + array->size * array->capacity) == p->header.last &&
            (p->header.last + size) <= p->header.end) {
            p->header.last += size;
            array->capacity += n;
        } else {
            uintptr_t capacity = 2 * ((n > array->capacity) ? n : array->capacity);
            void *arr = pmalloc(p, capacity * array->size, true);
            if (!arr) {
                return nullptr;
            }
            memcpy(arr, array->addr, array->used * array->size);
            array->addr = arr;
            array->capacity = capacity;
        }
    }
    void *addr = (unsigned char *) array->addr + array->size * array->used;
    array->used += n;

    return addr;
}

void ClearArray(Array *array) {
    array->used = 0;
}

void DestroyArray(Array *array) {
    if (((unsigned char *) array->addr + array->size * array->capacity) == array->pool->header.last) {
        array->pool->header.last -= array->size * array->capacity;
    }
    if ((unsigned char *) array + sizeof(Array) == array->pool->header.last) {
        array->pool->header.last = (unsigned char *) array;
    }
}
