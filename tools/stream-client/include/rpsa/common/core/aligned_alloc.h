/**
 * @file    aligned_alloc.h
 * @author  isnullxbh
 * @date    2019-03-19 13:37
 * @version 0.0.1
 */

#ifndef STREAMING_APPLICATION_COMMON_ALIGNED_ALLOC_H
#define STREAMING_APPLICATION_COMMON_ALIGNED_ALLOC_H

#include <cstddef>

void* aligned_alloc(size_t alignment, size_t size);
void  aligned_free(void* ptr);

#endif // STREAMING_APPLICATION_COMMON_ALIGNED_ALLOC_H
