#include "Platform.h"
#include "PlatformCTypes.h"

static ptime_t g_timeCount = 0;

_ptag void PlatformTimePassMs(void)
{
    g_timeCount++;
}

_ptag ptime_t PlatformTime(void)
{
    return g_timeCount;
}
