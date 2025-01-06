#include <cstdio>

#include "system/sysclk.h"

uint64 sys_get_hiresclk_ticks() {
    printf("sys_get_hiresclk_ticks()\n");
    // TODO: use performance.now?
    return 0;
}

uint64 sys_get_hiresclk_ticks_per_second() {
    printf("sys_get_hiresclk_ticks_per_second()\n");
    return 0;
}
