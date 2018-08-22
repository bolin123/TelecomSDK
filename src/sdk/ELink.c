#include "ELink.h"
#include "Util/json.h"
#include "Util/PList.h"

#define ELINK_SDK_VERSION "1.0.1"

#define ELINK_URL_ONLINE "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/startReportOnPower"
#define ELINK_URL_REPORT "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/regularReportingInterface"
#define ELINK_URL_EVENT  "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/eventTriggerReporting"
#define ELINK_URL_GET_CONFIG "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/configuring"
#define ELINK_URL_OTA_INFO   "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/requestFirmwareUpdate"
#define ELINK_URL_OTA_RESULT "http://180.168.75.148:30011/dxzdglapi/v1/deviceApi/firmwareDownloadResult"

typedef enum
{
    ELINK_MODE_START = 0,
    ELINK_MODE_ONLINE,
    ELINK_MODE_STOP,
}ELinkMode_t;

typedef enum
{
    ELINK_CMD_NONE = 0,
    ELINK_CMD_ONLINE_REPORT,
    ELINK_CMD_INTERVAL_REPORT,
    ELINK_CMD_EVENT_REPORT,
    ELINK_CMD_GET_CONFIG,
    ELINK_CMD_GET_OTA_INFO,
    ELINK_CMD_OTA_RESULT,
}ELinkCmd_t;

typedef struct ELinkData_st
{
    ELinkCmd_t cmd;
    char *contents;
    PLIST_ENTRY(struct ELinkData_st);
}ELinkData_t;

static const char *g_version = "01";
static ELinkMode_t g_mode = ELINK_MODE_STOP;
static ELinkCmd_t g_currentCmd = ELINK_CMD_NONE;
static ptime_t g_sendTime = 0;
static HTTPRequest_t *g_httpRequest = PNULL;
static ELinkData_t g_httpData;
static puint32_t g_reportInterval = 60;
static ptime_t g_lastReportTime;
static puint32_t g_getConfigInterval = 60;
static ptime_t g_lastGetconfigTime;
static char g_otaVersion[16] = "";

_ptag static void sendDone(void)
{
    plog("");
    g_currentCmd = ELINK_CMD_NONE;
    if(g_httpRequest)
    {
        HTTPRequestDestroy(g_httpRequest);
        g_httpRequest = PNULL;
    }
}

_ptag char *getUrl(ELinkCmd_t cmd)
{
    char *url = PNULL;
    switch(cmd)
    {
    case ELINK_CMD_ONLINE_REPORT:
        url = ELINK_URL_ONLINE;
        break;
    case ELINK_CMD_INTERVAL_REPORT:
        url = ELINK_URL_REPORT;
        break;
    case ELINK_CMD_EVENT_REPORT:
        url = ELINK_URL_EVENT;
        break;
    case ELINK_CMD_GET_CONFIG:
        url = ELINK_URL_GET_CONFIG;
        break;
    case ELINK_CMD_GET_OTA_INFO:
        url = ELINK_URL_OTA_INFO;
        break;
    case ELINK_CMD_OTA_RESULT:
        url = ELINK_URL_OTA_RESULT;
        break;
    default:
        break;
    }
    return url;
}
_ptag static pbool_t isLeapYear(puint32_t year)
{
    if(year && year % 4 == 0)
    {
        if(year % 100 == 0)
        {
            if(year % 400 != 0)
            {
                return pfalse;
            }
        }
        return ptrue;
    }

    return pfalse;
}


_ptag static char *utcToDateString(puint32_t utcTime)
{
    static char utcString[20] = "";
    puint32_t years = 1970;
    puint32_t days = utcTime / (24 * 60 * 60);
    puint8_t sec, min, hour;
    puint8_t timezone = 8;

    puint8_t monthInYear = 0;
    puint8_t monthDays;
    pbool_t isLeap = isLeapYear(years);
    //计算年
    while(ptrue)
    {
        switch(monthInYear)
        {
        case 0:
        case 2:
        case 4:
        case 6:
        case 7:
        case 9:
        case 11:
            monthDays = 31;
            break;

        case 1:
            monthDays = isLeap ? 29 : 28;
            break;
        default:
            monthDays = 30;
        }

        if(days < monthDays)
        {
            break;
        }

        monthInYear++;

        if(monthInYear == 12)
        {
            years++;
            monthInYear = 0;
            isLeap = isLeapYear(years);
        }

        days -= monthDays;
    }

    monthInYear += 1;

    puint32_t tmp = utcTime % (24 * 60 * 60);
    sec = (tmp % 60);
    tmp /= 60;
    min = tmp % 60;
    hour = (uint8_t)(tmp / 60);
    sprintf(utcString, "%04d-%02d-%02d %02d:%02d:%02d", years, monthInYear, days + 1, hour + timezone, min, sec);
    return utcString;
}

_ptag static void jsonHTTPSend(ELinkCmd_t cmd, json_item_t *msg)
{
    int len = json_item_eval_parse_text_len(msg);
    char *text = (char *)malloc(len + 1);

    text[0] = '\0';
    json_item_parse_to_text(msg, text);
    json_item_destroy(msg);

    plog("%s", text);

    ELinkData_t *elinkData = (ELinkData_t *)malloc(sizeof(ELinkData_t));
    if(elinkData)
    {
        elinkData->cmd = cmd;
        elinkData->contents = text;
        PListAdd(&g_httpData, elinkData);
    }
}

_ptag static void elinkGetOtaInfo(const char *did, char *version)
{
    json_item_t *cmd = json_item_create(JSON_OBJECT, PNULL);

    json_item_add_subitem(cmd, J_CREATE_I("code", 300));
    json_item_add_subitem(cmd, J_CREATE_S("gwid", did));

    json_item_t *update = json_item_create(JSON_OBJECT, "update");
    json_item_add_subitem(update, J_CREATE_S("ngwver", version));
    json_item_add_subitem(cmd, update);
    g_otaVersion[0] = '\0';
    strcpy(g_otaVersion, version);
    jsonHTTPSend(ELINK_CMD_GET_OTA_INFO, cmd);
}

_ptag static void httpRequestDataRecvCallback(HTTPRequest_t *request, const puint8_t *data, puint16_t len, HTTPRequestError_t error)
{
    int code;
    char *codestr;
    json_item_t *contents;
    json_item_t *config;
    json_item_t *url;
    PUpgradeNotice_t notice;
    PUpgradeInfo_t otaInfo;
    char *version, *reportInterval, *configInterval;
    PrivateCtx_t *ctx = (PrivateCtx_t *)request->userData;

    plog("return code %d", error);
    if(error == HTTP_REQ_ERROR_NONE)
    {
        plog("%s", (char *)data);
        contents = json_item_parse_from_text((char *)data);
        if(contents)
        {
            codestr = J_SUB_VALUE_BY_NAME_S(contents, "code");
            code = (int)strtol(codestr, PNULL, 10);
            if(code == 0)
            {
                switch(g_currentCmd)
                {
                case ELINK_CMD_ONLINE_REPORT:
                    g_mode = ELINK_MODE_ONLINE;
                    g_lastReportTime = PlatformTime();
                case ELINK_CMD_GET_CONFIG:
                    config = json_item_get_subitem_by_name(contents, "config");
                    if(config)
                    {
                        version = J_SUB_VALUE_BY_NAME_S(config, "ngwver");
                        if(version && strcmp(version, ELINK_SDK_VERSION) != 0)
                        {
                            elinkGetOtaInfo(ctx->did, version);
                        }
                        reportInterval = J_SUB_VALUE_BY_NAME_S(config, "nhbtime");
                        if(reportInterval)
                        {
                            g_reportInterval = (puint32_t)strtol(reportInterval, PNULL, 10);
                        }
                        configInterval = J_SUB_VALUE_BY_NAME_S(config, "configureAcquisitionCycle");
                        if(configInterval)
                        {
                            g_getConfigInterval = (puint32_t)strtol(configInterval, PNULL, 10);
                        }
                    }
                    g_lastGetconfigTime = PlatformTime();
                    break;
                case ELINK_CMD_GET_OTA_INFO:
                    url = json_item_get_subitem_by_name(contents, "URL");
                    if(url)
                    {
                        notice.did = ctx->did;
                        notice.infoNum = 1;
                        notice.info = &otaInfo;
                        otaInfo.type = 0;
                        otaInfo.name = PNULL;
                        otaInfo.version = g_otaVersion;
                        otaInfo.url = J_SUB_VALUE_BY_NAME_S(url, "gwURL");
                        PPrivateEventEmit(ctx, PEVENT_OTA_NOTICE, (void *)&notice);
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                plog("errcode = %d, %s", code, J_SUB_VALUE_BY_NAME_S(contents, "desc"));
            }
            json_item_destroy(contents);
        }
    }
    else
    {
        sendDone();
    }
}

_ptag static void httpDataSendHandle(PrivateCtx_t *ctx)
{
    ELinkData_t *eData = PListFirst(&g_httpData);

    if(eData && g_currentCmd == ELINK_CMD_NONE)
    {
        if(g_httpRequest)
        {
            HTTPRequestDestroy(g_httpRequest);
        }
        g_httpRequest = HTTPRequestCreate(getUrl(eData->cmd), "POST");
        if(g_httpRequest)
        {
            g_httpRequest->userData = (void *)ctx;
            g_httpRequest->dataRecvCb = httpRequestDataRecvCallback;
            HTTPRequestAddHeader(g_httpRequest, "content-type", "application/json;charset=utf-8");
            HTTPRequestSetData(g_httpRequest, eData->contents);
            HTTPRequestStart(g_httpRequest);
            g_currentCmd = eData->cmd;
            g_sendTime = PlatformTime();

            PListDel(eData);
            if(eData->contents)
            {
                free(eData->contents);
            }
            free(eData);
        }
    }
}

_ptag static void httpDataSendTimeout(void)
{
    if(g_currentCmd != ELINK_CMD_NONE && PTimeHasPast(g_sendTime, 20000))
    {
        sendDone();
    }
}

_ptag static void elinkGetConfig(const char *did)
{
    json_item_t *cmd = json_item_create(JSON_OBJECT, PNULL);
    plog("");

    if(cmd)
    {
        json_item_add_subitem(cmd, J_CREATE_I("code", 200));
        json_item_add_subitem(cmd, J_CREATE_S("gwid", did));
        jsonHTTPSend(ELINK_CMD_GET_CONFIG, cmd);
    }
}

_ptag void ELinkUpgradeResultReport(PrivateCtx_t *ctx, pbool_t success)
{
    json_item_t *result = json_item_create(JSON_OBJECT, PNULL);

    if(result)
    {
        if(success)
        {
            json_item_add_subitem(result, J_CREATE_I("code", 301));
            json_item_add_subitem(result, J_CREATE_S("desc", "success"));
        }
        else
        {
            json_item_add_subitem(result, J_CREATE_I("code", 999));
            json_item_add_subitem(result, J_CREATE_S("desc", "failed"));
        }
        json_item_add_subitem(result, J_CREATE_S("gwid", ctx->did));
        jsonHTTPSend(ELINK_CMD_OTA_RESULT, result);
    }
}

_ptag void ELinkSubdeviceOnoffLine(PrivateCtx_t *ctx, const char *did, pbool_t online)
{
    PMSubDevice_t *subDev = PPrivateGetSubDevice(ctx, did);

    if(subDev == PNULL || g_mode != ELINK_MODE_ONLINE)
    {
        return;
    }
    plog("");

    json_item_t *event = json_item_create(JSON_OBJECT, PNULL);
    if(online)
    {
        json_item_add_subitem(event, J_CREATE_I("code", 103));
    }
    else
    {
        json_item_add_subitem(event, J_CREATE_I("code", 104));
    }

    json_item_add_subitem(event, J_CREATE_S("gwid", ctx->did));
    json_item_add_subitem(event, J_CREATE_S("id", did));
    json_item_add_subitem(event, J_CREATE_B("ol", online));
    json_item_add_subitem(event, J_CREATE_I("lhr", PUtcTime()));

    json_item_t *st = json_item_create(JSON_OBJECT, "st");
    json_item_add_subitem(st, J_CREATE_S("fac", subDev->factoryName));
    json_item_add_subitem(st, J_CREATE_I("lqi", subDev->rssi));
    json_item_add_subitem(st, J_CREATE_I("batpt", subDev->battery));
    json_item_add_subitem(event, st);

    jsonHTTPSend(ELINK_CMD_EVENT_REPORT, event);
}

_ptag static void elinkIntervalInfoReport(PrivateCtx_t *ctx)
{
    json_item_t *info = json_item_create(JSON_OBJECT, PNULL);
    PMSubDevice_t *subDev;
    plog("");

    json_item_add_subitem(info, J_CREATE_I("code", 102));
    json_item_add_subitem(info, J_CREATE_S("gwid", ctx->did));
    json_item_add_subitem(info, J_CREATE_B("ol", ptrue));
    json_item_t *mst = json_item_create(JSON_OBJECT, "st");
    json_item_add_subitem(mst, J_CREATE_S("fac", ctx->factoryName));
    json_item_add_subitem(info, mst);

    json_item_t *device = json_item_create(JSON_ARRAY, "device");
    if(device)
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            json_item_t *item = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(item, J_CREATE_S("id", subDev->did));
            json_item_add_subitem(item, J_CREATE_I("lhr", subDev->hbTime));
            json_item_add_subitem(item, J_CREATE_B("ol", subDev->online));

            json_item_t *st = json_item_create(JSON_OBJECT, "st");
            json_item_add_subitem(st, J_CREATE_S("fac", subDev->factoryName));
            json_item_add_subitem(st, J_CREATE_I("lqi", subDev->rssi));
            json_item_add_subitem(st, J_CREATE_I("batpt", subDev->battery));
            json_item_add_subitem(item, st);

            json_item_add_subitem(device, item);
        }
        json_item_add_subitem(info, device);
    }

    jsonHTTPSend(ELINK_CMD_INTERVAL_REPORT, info);
}

_ptag static void elinkOnlineReport(PrivateCtx_t *ctx)
{
    char mac[18];
    char ip[16];
    json_item_t *info = json_item_create(JSON_OBJECT, PNULL);
    json_item_t *baseInfo = json_item_create(JSON_OBJECT, "baseInfo");

    plog("");
    if(baseInfo)
    {
        json_item_add_subitem(baseInfo, J_CREATE_S("VER", "01"));//编码协议版本号
        json_item_add_subitem(baseInfo, J_CREATE_S("CTEI", PGetCTEI()));
        json_item_add_subitem(baseInfo, J_CREATE_S("MAC", PGetMacAddr(mac)));
        json_item_add_subitem(baseInfo, J_CREATE_S("IP", PGetIpAddr(ip)));
        json_item_add_subitem(baseInfo, J_CREATE_S("LINK", "1"));
        json_item_add_subitem(baseInfo, J_CREATE_S("FWVER", ctx->version));
        json_item_add_subitem(baseInfo, J_CREATE_S("DATE", utcToDateString(PUtcTime())));
    }

    json_item_t *snap = json_item_create(JSON_OBJECT, "snap");
    if(snap)
    {
        json_item_add_subitem(snap, J_CREATE_I("code", 101));
        json_item_add_subitem(snap, J_CREATE_S("gwid", ctx->did));
        json_item_add_subitem(snap, J_CREATE_I("typeid", 1));
        json_item_add_subitem(snap, J_CREATE_S("modelid", ctx->model));//???
        json_item_t *mst = json_item_create(JSON_OBJECT, "st");
        json_item_add_subitem(mst, J_CREATE_S("fac", ctx->factoryName));
        json_item_add_subitem(snap, mst);
    }

    json_item_t *device = json_item_create(JSON_ARRAY, "device");
    PMSubDevice_t *subDev;
    if(device)
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            json_item_t *item = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(item, J_CREATE_S("id", subDev->did));
            json_item_add_subitem(item, J_CREATE_I("typeid", 0));
            json_item_add_subitem(item, J_CREATE_S("modelid", subDev->model));
            json_item_add_subitem(item, J_CREATE_B("ol", subDev->online));

            json_item_t *st = json_item_create(JSON_OBJECT, "st");
            json_item_add_subitem(st, J_CREATE_S("fac", subDev->factoryName));
            json_item_add_subitem(st, J_CREATE_I("lqi", subDev->rssi));
            json_item_add_subitem(st, J_CREATE_I("batpt", subDev->battery));
            json_item_add_subitem(item, st);
            json_item_add_subitem(device, item);
        }
        json_item_add_subitem(snap, device);
    }

    json_item_t *sdkversion = json_item_create(JSON_OBJECT, "sdkversion");
    json_item_add_subitem(sdkversion, J_CREATE_S("gwver", ELINK_SDK_VERSION));

    json_item_add_subitem(info, sdkversion);
    json_item_add_subitem(info, snap);
    json_item_add_subitem(info, baseInfo);

    jsonHTTPSend(ELINK_CMD_ONLINE_REPORT, info);
}

_ptag static void onlineReport(PrivateCtx_t *ctx)
{
    static ptime_t lastReportTime = 0;

    if(g_mode == ELINK_MODE_START)
    {
        if(lastReportTime == 0 || PTimeHasPast(lastReportTime, 20000))
        {
            elinkOnlineReport(ctx);
            lastReportTime = PlatformTime();
        }
    }
}

_ptag static void intervalProccess(PrivateCtx_t *ctx)
{
    if(g_mode == ELINK_MODE_ONLINE)
    {
        if(PTimeHasPast(g_lastReportTime, g_reportInterval * 60000))
        {
            elinkIntervalInfoReport(ctx);
            g_lastReportTime = PlatformTime();
        }

        if(PTimeHasPast(g_lastGetconfigTime, g_getConfigInterval * 60000))
        {
            elinkGetConfig(ctx->did);
            g_lastGetconfigTime = PlatformTime();
        }
    }
}

_ptag static void elinkClear(void)
{
    ELinkData_t *edata;

    PListForeach(&g_httpData, edata)
    {
        PListDel(edata);
        if(edata->contents)
        {
            free(edata->contents);
        }
        free(edata);
    }

    if(g_httpRequest)
    {
        HTTPRequestDestroy(g_httpRequest);
        g_httpRequest = PNULL;
    }
}

_ptag void ELinkStart(void)
{
    g_mode = ELINK_MODE_START;
    elinkClear();
}

_ptag void ELinkStop(void)
{
    g_mode = ELINK_MODE_STOP;
    elinkClear();
}

_ptag void ELinkLinkdown(void)
{
    elinkClear();
}

_ptag void ELinkInitialize(void)
{
    PListInit(&g_httpData);
}

_ptag void ELinkPoll(PrivateCtx_t *ctx)
{
    if(g_mode != ELINK_MODE_STOP && PIsLinkup())
    {
        onlineReport(ctx);
        httpDataSendHandle(ctx);
        httpDataSendTimeout();
        intervalProccess(ctx);
    }
}

