//
// Created by user on 19-4-9.
//

#include "MemoryPool.h"


PoolHeader *CreateMemoryPool(size_t size) {
    PoolHeader *p;
    //int err=posix_memalign()
    p = (PoolHeader *) palloc(size, false);
    if (p == nullptr) {
        return nullptr;
    }

    p->header.last = (unsigned char *) p + sizeof(PoolHeader);
    p->header.end = (unsigned char *) p + size;
    p->header.next = nullptr;
    p->header.failed = 0;

    size -= sizeof(PoolHeader);
    p->max = size < 4095 ? size : 4095;
    p->current = &p->header;
    p->large = nullptr;
    return p;
}

void DestroyMemoryPool(PoolHeader *pool) {
    BlockHeader *p, *t;
    PoolLarge *l;

    for (l = pool->large; l != nullptr; l = l->next) {
        if (l->data != nullptr) {
            free(l->data);
        }
    }
    for (p = &pool->header; p != nullptr; p = t) {
        t = p->next;
        free(p);
    }
}

void *palloc(size_t size, bool zero) {
    void *p = malloc(size);
    if (p == nullptr) {

        return nullptr;
    }
    if (zero) {
        memset(p, 0, size);
    }
    return p;
}

void *pmalloc(PoolHeader *pool, size_t size, bool Alignment) {
    unsigned char *m;
    BlockHeader *p;
    if (size <= pool->max) {
        p = pool->current;
        uintptr_t alignment = ALIGNMENT / 8;
        do {
            if (Alignment) {
                //Make the last log(2)(alignment) bits of p equal to 0, so m can be divisible by alignment, Boundary Alignment
                m = (unsigned char *) (((uintptr_t) p->last + (alignment - 1)) & ~(alignment - 1));
            } else {
                m = p->last;
            }
            if ((size_t) (p->end - m) >= size) {
                p->last = m + size;
                return m;
            }
            p = p->next;
        } while (p);
        return pmalloc_block(pool, size);
    }
    return pmalloc_large(pool, size);
}

void *pmalloc_large(PoolHeader *pool, size_t size) {
    void *p;
    PoolLarge *large;

    p = palloc(size, false);
    if (p == nullptr) {
        return nullptr;
    }
    int i = 0;
    for (large = pool->large; large != nullptr; large = large->next) {
        if (large->data == nullptr) {
            large->data = p;
            return p;
        }
        if (i > 3) {
            break;
        }
        ++i;
    }

    large = (PoolLarge *) pmalloc(pool, sizeof(PoolLarge), true);
    if (large == nullptr) {
        free(p);
        return nullptr;
    }

    large->data = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

void *pmalloc_block(PoolHeader *pool, size_t size) {
    unsigned char *m;
    size_t blocksize;
    BlockHeader *p, *newblock;

    blocksize = (size_t) (pool->header.end - (unsigned char *) pool);

    //posix_memalign()
    m = (unsigned char *) palloc(blocksize, false);
    if (m == nullptr) {
        return nullptr;
    }

    newblock = (BlockHeader *) m;
    newblock->end = m + blocksize;
    newblock->next = nullptr;
    newblock->failed = 0;
    m += sizeof(BlockHeader);
    uintptr_t alignment = ALIGNMENT / 8;
    m = (unsigned char *) (((uintptr_t) m + (alignment - 1)) & ~(alignment - 1));
    newblock->last = m + size;

    for (p = pool->current; p->next != nullptr; p = p->next) {
        if (p->failed > 4) {
            pool->current = p->next;
        }
        ++p->failed;
    }

    p->next = newblock;

    return m;
}

void pfree(PoolHeader *pool, void *p) {
    PoolLarge *l;
    for (l = pool->large; l != nullptr; l = l->next) {
        if (l->data == p) {
            free(l->data);
            l->data = nullptr;
            return;
        }
    }
    return;
}

void ResetMemoryPool(PoolHeader *pool) {
    PoolLarge *l;
    for (l = pool->large; l != nullptr; l = l->next) {
        free(l->data);
        l->data = nullptr;
    }

    pool->large = nullptr;
    pool->current = &pool->header;

    pool->header.last = (unsigned char *) pool + sizeof(PoolHeader);
    pool->header.failed = 0;

    BlockHeader *b = pool->header.next;
    while (b) {
        unsigned char *m = (unsigned char *) b;
        m += sizeof(BlockHeader);
        uintptr_t alignment = ALIGNMENT / 8;
        m = (unsigned char *) (((uintptr_t) m + (alignment - 1)) & ~(alignment - 1));
        b->last = m;
        b->failed = 0;

        b = b->next;
    }
}
