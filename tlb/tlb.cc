#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#include "arm_headers.h"

#define PAGE_SIZE 4096
#define NUM_PAGES 2048
#define BUFFER_SIZE (NUM_PAGES*PAGE_SIZE)
#define VICTIM_SIZE (16*PAGE_SIZE)

int main() {
    arm_v8_timing_init();
    // allocating a large array
    void (*func_ptr)(void *);
    system("/bin/ls .");
    uint8_t *victim_addr = (uint8_t *)malloc(sizeof(uint8_t) * VICTIM_SIZE);
    uint8_t *evict_buf = (uint8_t *)mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("victim function system addr: %p\n", system);
    printf("victim pointer addr: %p\n", victim_addr);
    printf("the purpose is to figure out the address of them\n");
    func_ptr = (void (*)(void*)) 0xffffbf5b3908;
    // func_ptr((void *)"/bin/ls");
    
    register uint8_t  *ptr  = NULL;
    register uint64_t page_idx       = 0;
    uint64_t evict_idx      = 0;
    uint64_t victim_idx     = 0;
    uint64_t page_offset    = 0;
    uint64_t pte_cacheline_offset = 0;
    uint64_t time_beg       = 0;
    uint64_t time_end       = 0;
    uint64_t *timing = (uint64_t *)malloc(sizeof(uint64_t) * 1024);
    // warm up: translate every page
    // and also make sure that they are allocated 
int j = 4;
while (--j) {
    for (int i = 0; i < 8; i++) {
        victim_idx = (i<< 12);
        victim_addr[victim_idx] = 0xbb;
    }
    for (int i = 0; i < NUM_PAGES; i++) {
        evict_idx = (i<< 12);
        evict_buf[evict_idx] = 0xbb;
    }
}
    // assuming the victim PTE is at a cache line
    // that has page offset as page_offset 
    arm_v8_memory_barrier();
for (pte_cacheline_offset = 0;  pte_cacheline_offset < 9; pte_cacheline_offset++) {
    for (page_offset = 0; page_offset < 64; page_offset++) {
        // access the victim to bring PTE in cache
        victim_idx = ((page_offset) & 0x3f) << 6;
        ptr = victim_addr + victim_idx + (pte_cacheline_offset << 12);
        // ptr = victim_addr + victim_idx + (pte_cacheline_offset << 7);
        // ptr = victim_addr + victim_idx;
        *ptr = 0x5a;
        // arm_v8_memory_barrier();
        // victim_addr[victim_idx] = 0x5a;
        for (page_idx = 0; page_idx < NUM_PAGES; page_idx++) {
            // if (((page_idx >> 3) & 0x3f) != page_offset) {
                // evict_idx = (page_idx << 12) + (page_offset << 6);
                // ptr = evict_buf + evict_idx;
                // *ptr = 0x5a;
            evict_buf[(page_idx << 12) + (page_offset << 6)] = 0x5a;
            // }
            // else {
            //     continue;
            // }
        }
        // testing now to see if PTE of this addresss is still there
        // ptr = victim_addr + victim_idx;
        arm_v8_memory_barrier();
        time_beg = arm_v8_get_timing();
        arm_v8_memory_barrier();
        *ptr = 0x5f;
        // victim_addr[victim_idx] = 0x5a;
        arm_v8_memory_barrier();
        time_end = arm_v8_get_timing();
        timing[page_offset] = time_end - time_beg;
    }

    // for (int i = 0; i < 64; i++) {
    //     printf("%d %lld\n", i, timing[i]);
    // }
    uint8_t sort[10];
    for (int i = 0; i < 10; i++) {
        sort[i] = 0;
    }
    int len = 0;
    for (int i = 1; i < 64; i++) {
        int tmp = len+1;
        for (int j = len; j >= 0; j--) {
            if (timing[i] > timing[sort[j]]) {
                tmp = j;
            }
        }
        if (tmp < 10) {
            if (len < 9) len++;
            for (int j = len; j > tmp; j--) {
                sort[j] = sort[j-1];
            }
            sort[tmp] = i;
        }
    }
    uint64_t ans = (((uint64_t) ptr >> 12) >> 3) & 0x3f;
    printf("correct bit should be 0x%x: %d\n", ans, ans);
    for (int i = 0; i < 10; i++) {
        printf("%d %llu; ", sort[i], timing[sort[i]]);
    }
    printf("\n");
}
    arm_v8_timing_terminate();
   

    return 0;
}
