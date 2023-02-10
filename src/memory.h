#ifndef __H_MEMORY__
#define __H_MEMORY__

#include <stdint.h>
#include <stddef.h>

void* malloc_himem(size_t size, int32_t use_high_memory);
void free_himem(void* ptr, int32_t use_high_memory);
int32_t resize_himem(void* ptr, size_t size, int32_t use_high_memory);

#endif