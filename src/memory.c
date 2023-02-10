#include <stdint.h>
#include <stddef.h>
#include <iocslib.h>
#include <doslib.h>
#include "memory.h"

// allocate high memory
static void* __malloc_himem(size_t size) {

    struct REGS in_regs = { 0 };
    struct REGS out_regs = { 0 };

    in_regs.d0 = 0xF8;      // IOCS _HIMEM
    in_regs.d1 = 1;         // HIMEM_MALLOC
    in_regs.d2 = size;

    TRAP15(&in_regs, &out_regs);

    uint32_t rc = out_regs.d0;

    return (rc == 0) ? (void*)out_regs.a1 : NULL;
}

// free high memory
static void __free_himem(void* ptr) {
    
    struct REGS in_regs = { 0 };
    struct REGS out_regs = { 0 };

    in_regs.d0 = 0xF8;      // IOCS _HIMEM
    in_regs.d1 = 2;         // HIMEM_FREE
    in_regs.d2 = (size_t)ptr;

    TRAP15(&in_regs, &out_regs);
}

// resize high memory
int __resize_himem(void* ptr, size_t size) {

    struct REGS in_regs = { 0 };
    struct REGS out_regs = { 0 };

    in_regs.d0 = 0xF8;          // IOCS _HIMEM
    in_regs.d1 = 4;             // HIMEM_RESIZE
    in_regs.d2 = (size_t)ptr;
    in_regs.d3 = size;

    TRAP15(&in_regs, &out_regs);
  
    return out_regs.d0;
}

// allocate main memory
static void* __malloc_mainmem(size_t size) {
  uint32_t addr = MALLOC(size);
  return (addr >= 0x81000000) ? NULL : (void*)addr;
}

// free main memory
static void __free_mainmem(void* ptr) {
  if (ptr == NULL) return;
  MFREE((uint32_t)ptr);
}

// resize main memory
static int32_t __resize_mainmem(void* ptr, size_t size) {
  return SETBLOCK((uint32_t)ptr, size);
}

// allocate memory
void* malloc_himem(size_t size, int32_t use_high_memory) {
    return use_high_memory ? __malloc_himem(size) : __malloc_mainmem(size);
}

// free memory
void free_himem(void* ptr, int32_t use_high_memory) {
    if (use_high_memory) {
        __free_himem(ptr);
    } else {
        __free_mainmem(ptr);
    }
}

// resize memory
int32_t resize_himem(void* ptr, size_t size, int32_t use_high_memory) {
    return use_high_memory ? __resize_himem(ptr, size) : __resize_mainmem(ptr, size);
}