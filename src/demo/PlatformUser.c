#include "PlatformUser.h"
#include "UserTypes.h"
#include "Platform.h"
#include "PlatformMc.h"
#include <time.h>

#ifdef __WIN__
#include <Windows.h>
#endif

static ustime_t g_time = 0;
static PlatformCtx_t *g_ctx;

bool UserPhyIsLinkup(void)
{
    return true;
}

uint32_t PlatformUserUtcTime(void)
{
    return (uint32_t)time(NULL);
}

ustime_t PUserTime(void)
{
    return g_time;
}

uint32_t UserGetTickCount(void)
{
    return (uint32_t)GetTickCount();
}

/*
void PlatformUserTimePass(int ms)
{
    PlatformTimePassMs();
    g_time += ms;
}
*/

void PlatformUserInit(void)
{
    PPropertyInfo_t pInfo;
    char *devID = "00B0070103010010014293641078460438";
    char *devPin = "18596809735815542467406343584693";

    PlatformInitialize();
    g_ctx = PlatformCtxCreate();
    if(g_ctx)
    {
        PlatformSetDevid(g_ctx, devID);
        PlatformSetPin(g_ctx, devPin);
        PlatformSetModel(g_ctx, "1001");
        PlatformSetDeviceVersion(g_ctx, "001.000.000.001");

        pInfo.appid    = 1;
        pInfo.sid      = 0;
        pInfo.name     = "ACTION";
        pInfo.isText   = true;
        pInfo.readonly = false;
        PlatformPropertyRegister(g_ctx, &pInfo);
        //PlatformPropertyRegist(g_ctx, 2, 1, "POWER", false, false);
        //PlatformPropertyRegist(g_ctx, 3, 2, "POWER", false, false);
        //PlatformPropertyRegist(g_ctx, 4, 3, "POWER", false, false);
        PlatformStart(g_ctx);


        PlatformPropertySetTextValue(g_ctx, 1, "VOICECAST");
        //PlatformPropertySetNumValue(g_ctx, 2, 1);
        //PlatformPropertySetNumValue(g_ctx, 3, 0);
        //PlatformPropertySetNumValue(g_ctx, 4, 1);

        PlatformResourceRegister(g_ctx, devID, "KEY_LIST", 1, 2);

        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 0, "KEY_ID", false);
        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 0, "KEY_TYPE", false);
        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 0, "KEY_MODE", false);

        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 1, "KEY_ID", false);
        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 1, "KEY_TYPE", false);
        PlatformResourceInfoRegister(g_ctx, devID, "KEY_LIST", 1, "KEY_MODE", false);

        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 0, "KEY_ID", 1);
        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 0, "KEY_TYPE", 2);
        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 0, "KEY_MODE", 3);

        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 1, "KEY_ID", 2);
        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 1, "KEY_TYPE", 3);
        PlatformResourceInfoSetNumValue(g_ctx, devID, "KEY_LIST", 1, "KEY_MODE", 4);

        PlatformResourceRegister(g_ctx, devID, "CONSUM", 0, 1);
        PlatformResourceInfoRegister(g_ctx, devID, "CONSUM", 0, "COUSUM_ID", true);
        PlatformResourceInfoRegister(g_ctx, devID, "CONSUM", 0, "COUSUM_NAME", true);
        PlatformResourceInfoRegister(g_ctx, devID, "CONSUM", 0, "REMAIN_LIFE", false);

        PlatformResourceInfoSetTextValue(g_ctx, devID, "CONSUM", 0, "COUSUM_ID", "3456789");
        PlatformResourceInfoSetTextValue(g_ctx, devID, "CONSUM", 0, "COUSUM_NAME", "MAIN_FILTER");
        PlatformResourceInfoSetNumValue(g_ctx, devID, "CONSUM", 0, "REMAIN_LIFE", 87);
    }
}

void PlatformUserPoll(void)
{
    PlatformPoll(g_ctx);
}

