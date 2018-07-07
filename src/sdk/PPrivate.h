#ifndef PPRIVATE_H
#define PPRIVATE_H

#include "PlatformCTypes.h"
#include "Util/PList.h"

typedef enum
{
    PSERVER_STATUS_IDLE = 0,
    PSERVER_STATUS_LOGIN_START,
    PSERVER_STATUS_LOGIN_DONE,
    PSERVER_STATUS_CONNECT_START,
    PSERVER_STATUS_CONNECT_DONE,
}PServerStatus_t;

typedef struct PCtx_st
{
    //Ìí¼Ó·¢ÏÖ×´Ì¬
    pbool_t startAddDiscover;
    ptime_t startAddDiscoverTime;

    //PServerStatus_t serverStatus;
    ptime_t lastServerStatusTime;
}PCtx_t;

typedef struct PSubCtx_st
{
    pbool_t online;
    char did[PLATFORM_DEVID_LEN + 1];
    char pin[PLATFORM_PIN_LEN + 1];
    PLIST_ENTRY(struct PSubCtx_st);
}PSubCtx_t;

ptime_t PlatformTime(void);
#define PTimeDiff(new, old) ((new) - (old))
#define PTimeDiffCurrent(oldTime) PTimeDiff(PlatformTime(), (oldTime))
#define PTimeHasPast(oldTime, pastTime) (PTimeDiffCurrent((oldTime)) > (pastTime))


const char *PPrivateGetToken(void);
const char *PPrivateGetSessionkey(void);
void PPrivateSetToken(const char *token);
void PPrivateSetSessionkey(const char *key);

#endif // !PPRIVATE_H
