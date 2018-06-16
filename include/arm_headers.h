
#if defined(__aarch64__)
#include "armv8_timing.h"
#define arm_access_memory(x)    arm_v8_access_memory(x)
#define arm_memory_barrier()    arm_v8_memory_barrier()
#define arm_timing_init(x)      arm_v8_timing_init()
#define arm_timing_terminate()  arm_v8_timing_terminate()
#define arm_get_timing()        arm_v8_get_timing()
#else
#include "armv7_timing.h"
#define arm_access_memory(x)    arm_v7_access_memory(x)
#define arm_memory_barrier()    arm_v7_memory_barrier()
#define arm_timing_init(x)      arm_v7_timing_init(x)
#define arm_timing_terminate()  arm_v7_timing_terminate()
#define arm_get_timing()        arm_v7_get_timing()
#endif

