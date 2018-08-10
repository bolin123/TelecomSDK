#include "Platform.h"
#include "PPrivate.h"
#include "PlatformMc.h"
#include "PlatformOTA.h"
#include "PropertyManager.h"
#include "ConnectionManager.h"

//static PlatformEventHandle_t g_eventHandle;
#if 0
_ptag int PlatformResourceInfoSetNumValue(PlatformCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, int value)
{
    return PMResourceInfoSetNumValue(ctx->private, did, rscName, infoID, infoName, value);
}

_ptag int PlatformResourceInfoSetTextValue(PlatformCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, const char *value)
{
    return PMResourceInfoSetTextValue(ctx->private, did, rscName, infoID, infoName, value);
}

_ptag int PlatformResourceInfoRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, pbool_t isText)
{
    return PMResourceInfoRegister(ctx->private, did, rscName, infoID, infoName, isText);
}

_ptag int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *name, puint16_t sid, puint16_t infoNum)
{
    return PMResourceRegister(ctx->private, did, name, sid, infoNum);
}
#endif

_ptag void PlatformTTS(PlatformCtx_t *ctx, const char *did, PTTSParameter_t *param)
{
    PlatformMcTTS(ctx->private, did, param);
}

_ptag void PlatformErrorReport(PlatformCtx_t *ctx, const char *did, PErrorReport_t *error)
{
    PlatformMcErrReport(ctx->private, did, error);
}

_ptag void PlatformStatusAlarm(PlatformCtx_t *ctx, const char *did, PStatusAlarm_t *alarm)
{
    PlatformMcStatusAlarm(ctx->private, did, alarm);
}

_ptag void PlatformVoiceControl(PlatformCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum)
{
    PlatformMcVoiceControl(ctx->private, did, param, data, isLast, ext, extNum);
}

_ptag void PlatformEventInfoReport(PlatformCtx_t *ctx, const char *did, PEventInfo_t *event)
{
    PlatformMcEventInfoReport(ctx->private, did, event);
}

_ptag int PlatformResourceItemSet(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum)
{
    return PMResourceItemSet(ctx->private, did, rscName, id, value, valNum);
}

_ptag int PlatformResourceItemDel(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id)
{
    return PMResourceItemDel(ctx->private, did, rscName, id);
}

_ptag int PlatformResourceItemAdd(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum)
{
    return PMResourceItemAdd(ctx->private, did, rscName, id, value, valNum);
}

_ptag int PlatformResourceKeywordRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *keyword, ptBool_t valueIsText)
{
    return PMResourceKeywordRegister(ctx->private, did, rscName, keyword, valueIsText);
}

_ptag int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *idName, unsigned short serialId)
{
    return PMResourceRegister(ctx->private, did, rscName, idName, serialId);
}

_ptag int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value)
{
    return PMPropertySetTextValue(ctx->private, did, appid, value);
}

_ptag int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value)
{
    return PMPropertySetNumValue(ctx->private, did, appid, value);
}

_ptag void PlatformStartOTA(PlatformCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo)
{
    PlatformOTAStartDownload(ctx->private, did, upgradeInfo);
}

_ptag void PlatformOTAResult(PlatformCtx_t *ctx, ptBool_t success)
{
    plog("success %d", success);
    PlatformMcOTAResultReport(ctx->private, success);
    PlatformOTAStopDownlaod(ctx->private);
}

_ptag int PlatformPropertyRegister(PlatformCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo)
{
    return PMPropertyRegister(ctx->private, did, pInfo);
}

_ptag void PlatformSetModuleVersion(PlatformCtx_t *ctx, const char *did, const char *name, const char *version)
{
    PMModuleSetVersion(ctx->private, did, name, version);
}

_ptag void PlatformSetDeviceInfo(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version)
{
    strncpy(ctx->private->did, did, PLATFORM_DEVID_LEN);
    strncpy(ctx->private->pin, pin, PLATFORM_PIN_LEN);
    strncpy(ctx->private->model, model, PLATFORM_MODEL_LEN);
    strncpy(ctx->private->version, version, PLATFORM_VERSION_LEN);
}

_ptag void PlatformStart(PlatformCtx_t *ctx)
{
    CMStart(ctx->private);
}

_ptag int PlatformSubDeviceUnbind(PlatformCtx_t *ctx, const char *did)
{
    PPrivateSubDeviceDel(ctx->private, did);
    PlatformMcSubDeviceUnbindReport(ctx->private, did);
    return 0;
}

_ptag int PlatformSubDeviceOnOffline(PlatformCtx_t *ctx, const char *did, ptBool_t online)
{
    return PlatformMcSubDeviceOnOffline(ctx->private, did, online);
}

_ptag int PlatformSubDeviceRegister(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version)
{
    return PPrivateSubDeviceRegister(ctx->private, did, pin, model, version);
}

_ptag ptBool_t PlatformServerConnected(PlatformCtx_t *ctx)
{
    return CMServerConnected(ctx->private);
}

_ptag void PlatformEventRegister(PlatformCtx_t *ctx, PlatformEventHandle_t handle)
{
    if(ctx && ctx->private)
    {
        ctx->private->eventHandle = handle;
    }
}

_ptag PlatformCtx_t *PlatformCtxCreate(void)
{
    PlatformCtx_t *ctx = (PlatformCtx_t *)malloc(sizeof(PlatformCtx_t));
    if(ctx)
    {
        ctx->private = PPrivateCreate();
        if(ctx->private)
        {
            return ctx;
        }
        else
        {
            free(ctx);
        }
    }
    return PNULL;
}

_ptag void PlatformInitialize(void)
{
    PPrivateInitialize();
    CMInitialize();
    PlatformMcInitialize();
    HTTPRequestInitialize();
}

_ptag void PlatformPoll(PlatformCtx_t *ctx)
{
    PlatformMcPoll(ctx->private);
    CMPoll(ctx->private);
    HTTPRequestPoll();
}


