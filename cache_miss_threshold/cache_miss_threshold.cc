#include <time.h>
#include <stdlib.h>

#include "arm_headers.h"

#define ARR_SIZE (1024*1024*8)
#define PAGE_SIZE (4096/sizeof(uint32_t))

int main() {
    srand(time(NULL));
    uint32_t array[1024];
    uint32_t *marr = static_cast<uint32_t*>(malloc(sizeof(uint32_t) * ARR_SIZE)); 
    volatile void *ptr = &array;
    uint64_t tic = 0, toc = 0;
    arm_timing_init(false);
    for (int i = 0; i < 10; i++) {
        tic = arm_get_timing();
        arm_memory_barrier();

        arm_access_memory((void *)ptr);

        arm_memory_barrier();
        toc = arm_get_timing();
        printf("hit: %llu\n", toc - tic);
    }
   
    // this prevents TLB Miss, etc
    uint32_t ppn = rand() % (ARR_SIZE / PAGE_SIZE);
    uint32_t val = marr[ppn * PAGE_SIZE];
    arm_access_memory((void *)(&marr[ppn * PAGE_SIZE]));

    ptr = marr + (ppn * PAGE_SIZE + rand() % PAGE_SIZE);
    for (int i = 0; i < 10; i++) {
        tic = arm_get_timing();
        arm_memory_barrier();

        arm_access_memory((void *)ptr);

        arm_memory_barrier();
        toc = arm_get_timing();
        ptr = marr + (ppn * PAGE_SIZE + rand() % PAGE_SIZE);
        printf("random access %p: %llu\n", ptr, toc - tic);
    }

    arm_timing_terminate();
}
