#ifndef PLATFORM_USER_H
#define PLATFORM_USER_H

#include "PC/UserTypes.h"

ustime_t PUserTime(void);
//void PlatformUserTimePass(int ms);
void PlatformUserInit(void);
void PlatformUserPoll(void);
#endif
