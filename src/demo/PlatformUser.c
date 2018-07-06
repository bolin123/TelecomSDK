#include "PlatformUser.h"
#include "SysTypes.h"
#include "Platform.h"
#include "PlatformMc.h"
#include <time.h>

vtuint32_t PlatformUserUtcTime(void)
{
    //return (vtuint32_t)time(NULL);
    return 1516957510;
}

void PlatformUserTimePass(int ms)
{
    PlatformTimePassMs();
}

void PlatformUserInit(void)
{
    PlatformMcLogin("MC93c9c2d8ac435db6019ba619d6a77d", "111.222.333.444", 12345);
}

void PlatformUserPoll(void)
{
}
