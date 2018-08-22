#include "UserTypes.h"
#include "../Wifi.h"

ROM_FUNC char *PGetCTEI(void)
{
    return "123456789012345";
}

ROM_FUNC char *PGetMacAddr(char *mac)
{
    return WifiGetMac();
}

ROM_FUNC char *PGetIpAddr(char *ip)
{
    return WifiGetIp(ip);
}

ROM_FUNC unsigned char PIsLinkup(void)
{
    return WifiConnected();
}

extern unsigned int g_utcTime;
ROM_FUNC unsigned int PUtcTime(void)
{
    return g_utcTime;
}

extern uint32_t SysGetTimeMs(void);
ROM_FUNC unsigned int PlatformTime(void)
{
    return SysGetTimeMs();
}




