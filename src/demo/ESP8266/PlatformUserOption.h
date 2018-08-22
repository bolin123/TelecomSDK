#ifndef PLATFORM_USER_OPTION_H
#define PLATFORM_USER_OPTION_H

#include "UserTypes.h"

#define pprintf uPrintf
#define _ptag __attribute__((section(".irom0.text")))

#define PLATFORM_SERVER_URL "180.100.133.131"
#define PLATFORM_SERVER_PORT 9017

#endif // !PLATFORM_USER_OPTION_H
