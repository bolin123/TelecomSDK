#ifndef PLATFORM_USER_OPTION_H
#define PLATFORM_USER_OPTION_H

#include "SysTypes.h"

vtuint32_t PlatformUserUtcTime(void);
#define pprintf printf
#define SysUtcTime PlatformUserUtcTime
#define _ptag 

#define PLATFORM_SERVER_URL "180.100.133.131"
#define PLATFORM_SERVER_PORT 9017
#define PLATFORM_DEVICE_VERSION "000.000.000.001"

#endif // !PLATFORM_USER_OPTION_H
