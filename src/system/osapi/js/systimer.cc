#include <sys/time.h>

#include "system/sysclk.h"

uint64 sys_get_hiresclk_ticks() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (uint64(tv.tv_sec) * 1000000) + tv.tv_usec;
}

uint64 sys_get_hiresclk_ticks_per_second() {
    return 1000000;
}
