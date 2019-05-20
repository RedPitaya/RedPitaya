/**
 * @file    aligned_alloc.cpp
 * @author  isnullxbh
 * @date    2019-03-19 13:38
 * @version 0.0.1
 */

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "rpsa/common/core/aligned_alloc.h"

#ifndef align_up
#define align_up(num, align) \
	(((num) + ((align) - 1)) & ~((align) - 1))
#endif

typedef uint16_t offset_t;
#define PTR_OFFSET_SZ sizeof(offset_t)

void* aligned_alloc(size_t align, size_t size) {
    void * ptr = nullptr;
    assert((align & (align - 1)) == 0);

    if(align && size) {
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void * p = malloc(size + hdr_size);

        if(p) {
            ptr = (void *) align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);
            *((offset_t *)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);
        }
    }

    return ptr;
}

void aligned_free(void* ptr) {
    assert(ptr);
    offset_t offset = *((offset_t *)ptr - 1);
    void * p = (void *)((uint8_t *)ptr - offset);
    free(p);
}
