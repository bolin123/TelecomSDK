#include "PlatformOTA.h"

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static void startHttp(PrivateCtx_t *ctx);

/*******************************************************************************
 * Function Defination
 ******************************************************************************/

_ptag void httpRequestDataRecvCallback(HTTPRequest_t *request, const puint8_t *data, puint16_t len, HTTPRequestError_t error)
{
    PrivateCtx_t *ctx = request->userData;

    if(error == HTTP_REQ_ERROR_SUCCESS)
    {
        //未下载完成继续下载
        if(ctx->ota.totalRecvLen < ctx->ota.fileSize)
        {
            //delay startHttp
            startHttp(ctx);
            return;
        }

        plog("ota download end");

        PPrivateEventEmit(ctx, PEVENT_OTA_FINISH, (void *)ptrue);
    }
    else if(error == HTTP_REQ_ERROR_FAIL)
    {
        //下载失败最多重试10次
        if(ctx->ota.retriesCnt < 10)
        {
            ctx->ota.retriesCnt++;
            startHttp(ctx);
        }
        else
        {
            plog("ota download fail");

            PPrivateEventEmit(ctx, PEVENT_OTA_FINISH, (void *)pfalse);
        }
    }
    else
    {
        if(ctx->ota.fileSize == 0)
        {
            ctx->ota.fileSize = request->respContentLength;
        }

        ctx->ota.totalRecvLen += len;

        //int progress = ctx->ota.fileSize == 0 ? 0 : (ctx->ota.totalRecvLen * 100 / ctx->ota.fileSize);

        //数据
        POTADataArgs_t arg;
        arg.data = (char *)data;
        arg.datalen = len;
        arg.fileSize = ctx->ota.fileSize;
        PPrivateEventEmit(ctx, PEVENT_OTA_DATA, &arg);
    }
}

_ptag static void startHttp(PrivateCtx_t *ctx)
{
    if(ctx->ota.request)
    {
        HTTPRequestDestroy(ctx->ota.request);
    }
    ctx->ota.request = HTTPRequestCreate(ctx->ota.url, HTTP_REQ_METHOD_GET);
    ctx->ota.request->userData = ctx;
    ctx->ota.request->dataRecvCb = httpRequestDataRecvCallback;


    //已获取文件长度，则使用断点续传
    if(ctx->ota.fileSize != 0)
    {
        char text[20];

#ifdef EMW5088
        uint32_t targetpos = ctx->ota.totalRecvLen + KB(8) - 1;
        if(targetpos >= ctx->ota.fileSize)
        {
            targetpos = ctx->ota.fileSize - 1;
        }
        sprintf(text, "bytes=%d-%d", ctx->ota.totalRecvLen, targetpos);
#else
        sprintf(text, "bytes=%d-", ctx->ota.totalRecvLen);
#endif
        HTTPRequestAddHeader(ctx->ota.request, "Range", text);
        ctx->ota.request->timeout = 5000;
    }

    HTTPRequestStart(ctx->ota.request);
}

_ptag void PlatformOTAStartDownload(PrivateCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo)
{
    if(ctx->ota.startDownload)
    {
        plog("err, has start.");
        return;
    }

    if(ctx->ota.did)
    {
        free(ctx->ota.did);
        ctx->ota.did = PNULL;
    }

    ctx->ota.did = malloc(strlen(did) + 1);
    if(ctx->ota.did)
    {
        ctx->ota.did[0] = '\0';
        strcpy(ctx->ota.did, did);
    }

    //url
    if(ctx->ota.url)
    {
        free(ctx->ota.url);
        ctx->ota.url = PNULL;
    }

    ctx->ota.url = malloc(strlen(upgradeInfo->url) + 1);
    if(ctx->ota.url)
    {
        ctx->ota.url[0] = '\0';
        strcpy(ctx->ota.url, upgradeInfo->url);
    }

    //version
    if(ctx->ota.version)
    {
        free(ctx->ota.version);
        ctx->ota.version = PNULL;
    }
    ctx->ota.version = malloc(strlen(upgradeInfo->version) + 1);
    if(ctx->ota.version)
    {
        ctx->ota.version[0] = '\0';
        strcpy(ctx->ota.version, upgradeInfo->version);
    }

    //name
    if(ctx->ota.name)
    {
        free(ctx->ota.name);
        ctx->ota.name = PNULL;
    }
    if(upgradeInfo->name)
    {
        ctx->ota.name = malloc(strlen(upgradeInfo->name) + 1);
        if(ctx->ota.name)
        {
            ctx->ota.name[0] = '\0';
            strcpy(ctx->ota.name, upgradeInfo->name);
        }
    }

    plog("start ota, type=%d url=%s", upgradeInfo->type, upgradeInfo->url);

    ctx->ota.startDownload = ptrue;
    ctx->ota.retriesCnt = 0;
    ctx->ota.totalRecvLen = 0;
    ctx->ota.fileSize = 0;
    ctx->ota.type = upgradeInfo->type;
    startHttp(ctx);
}

_ptag void PlatformOTAStopDownlaod(PrivateCtx_t *ctx)
{
    ctx->ota.startDownload = pfalse;

    if(ctx->ota.did)
    {
        free(ctx->ota.did);
        ctx->ota.did = PNULL;
    }
    if(ctx->ota.version)
    {
        free(ctx->ota.version);
        ctx->ota.version = PNULL;
    }
    if(ctx->ota.url)
    {
        free(ctx->ota.url);
        ctx->ota.url = PNULL;
    }
    if(ctx->ota.name)
    {
        free(ctx->ota.name);
        ctx->ota.name = PNULL;
    }

    if(ctx->ota.request)
    {
        HTTPRequestDestroy(ctx->ota.request);
        ctx->ota.request = PNULL;
    }
}

