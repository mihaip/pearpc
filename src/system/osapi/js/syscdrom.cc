#include <cstdio>

#include "io/ide/cd.h"
#include "system/syscdrom.h"

CDROMDevice* createNativeCDROMDevice(const char* device_name, const char* image_name)
{
    // TODO: mountable CD-ROM device support
    printf("createNativeCDROMDevice()\n");
    return NULL;
}
