#include <emscripten.h>

#include "system/sysclk.h"

uint64 sys_get_hiresclk_ticks() {
    double performanceNow = EM_ASM_DOUBLE({
        return performance.now();
    });
    return uint64(performanceNow * 1000.0);
}

uint64 sys_get_hiresclk_ticks_per_second() {
    return 1000000;
}
