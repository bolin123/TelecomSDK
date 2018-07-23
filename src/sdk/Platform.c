#include "Platform.h"
#include "PPrivate.h"
#include "PlatformMc.h"
#include "PropertyManager.h"
#include "ConnectionManager.h"

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

_ptag int PlatformPropertySetTextValue(PlatformCtx_t *ctx, unsigned short appid, const char *value)
{
    return PMPropertySetTextValue(&ctx->private->properties, appid, value);
}

_ptag int PlatformPropertySetNumValue(PlatformCtx_t *ctx, unsigned short appid, unsigned int value)
{
    return PMPropertySetNumValue(&ctx->private->properties, appid, value);
}

_ptag int PlatformPropertyRegister(PlatformCtx_t *ctx, PPropertyInfo_t *pInfo)
{
    return PMPropertyRegister(&ctx->private->properties, pInfo);
}

_ptag void PlatformSetDeviceVersion(PlatformCtx_t *ctx, const char *version)
{
    strncpy(ctx->private->version, version, PLATFORM_VERSION_LEN);
}

_ptag void PlatformSetPin(PlatformCtx_t *ctx, const char *pin)
{
    strncpy(ctx->private->pin, pin, PLATFORM_PIN_LEN);
}

_ptag void PlatformSetDevid(PlatformCtx_t *ctx, const char *devid)
{
    strncpy(ctx->private->did, devid, PLATFORM_DEVID_LEN);
}

_ptag void PlatformSetModel(PlatformCtx_t *ctx, const char *model)
{
    strncpy(ctx->private->model, model, PLATFORM_MODEL_LEN);
}

_ptag void PlatformStart(PlatformCtx_t *ctx)
{
    CMStart(ctx->private);
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


