#include "Platform.h"
#include "PPrivate.h"
#include "PlatformMc.h"
#include "PropertyManager.h"
#include "ConnectionManager.h"

//static PlatformEventHandle_t g_eventHandle;

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

_ptag int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value)
{
    return PMPropertySetTextValue(ctx->private, did, appid, value);
}

_ptag int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value)
{
    return PMPropertySetNumValue(ctx->private, did, appid, value);
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
    return CMServerConnected(ctx);
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
}

_ptag void PlatformPoll(PlatformCtx_t *ctx)
{
    PlatformMcPoll(ctx->private);
    CMPoll(ctx->private);
}


