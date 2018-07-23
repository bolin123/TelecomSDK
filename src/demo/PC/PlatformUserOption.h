#ifndef PLATFORM_USER_OPTION_H
#define PLATFORM_USER_OPTION_H

#include "UserTypes.h"

bool UserPhyIsLinkup(void);
#define PIsLinkup UserPhyIsLinkup

uint32_t PlatformUserUtcTime(void);
#define PUtcTime PlatformUserUtcTime

uint32_t UserGetTickCount(void);
#define PlatformTime UserGetTickCount

#define pprintf printf
#define _ptag

#define PLATFORM_SERVER_URL "180.100.133.131"
#define PLATFORM_SERVER_PORT 9017
//#define PLATFORM_DEVICE_VERSION "000.000.000.001"

#endif // !PLATFORM_USER_OPTION_H
