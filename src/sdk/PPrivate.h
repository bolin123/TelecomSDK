#ifndef PPRIVATE_H
#define PPRIVATE_H

#include "PlatformCTypes.h"
#include "Adapter/PSocket.h"
#include "Adapter/PAdapter.h"
#include "Util/PList.h"
#include "Util/HTTPRequest.h"
#include "Platform.h"

typedef enum
{
    RESOURCE_MODE_ADD = 1,
    RESOURCE_MODE_SET,
    RESOURCE_MODE_DEL,
}ResourceMode_t;

typedef enum
{
    CM_SERVER_STATUS_IDLE = 0,
    CM_SERVER_STATUS_LS_CONNECTING,
    CM_SERVER_STATUS_LS_CONNECTED,
    CM_SERVER_STATUS_CS_CONNECTING,
    CM_SERVER_STATUS_CS_CONNECTED,
    CM_SERVER_STATUS_ONLINE,
}CMServerStatus_t;

typedef enum
{
    PROPERTY_TYPE_TEXT = 0,
    PROPERTY_TYPE_NUM = 1,
}PropertyType_t;

typedef union
{
    pint32_t num;
    char *text;
}PropertyValue_t;

typedef struct ResourceNode_st
{
    char *name;
    PropertyType_t type;
    PropertyValue_t value;
    PLIST_ENTRY(struct ResourceNode_st);
}ResourceNode_t;

typedef struct ResourceItems_st
{
    puint16_t idValue;
    ResourceMode_t mode; //1:add, 2:change, 3:del
    pbool_t changed;
    ResourceNode_t node;
    PLIST_ENTRY(struct ResourceItems_st);
}ResourceItems_t;

#define RESOURCE_DEFAULT_KEY_LEN 10
typedef struct ResourceInfo_st
{
    char *rscName;
    char *idName;
    char *key[RESOURCE_DEFAULT_KEY_LEN];
    pbool_t keyValueIsText[RESOURCE_DEFAULT_KEY_LEN];
    puint8_t keyNum;
    puint16_t serialID;
    ResourceItems_t items;
    PLIST_ENTRY(struct ResourceInfo_st);
}ResourceInfo_t;

/*设备属性信息*/
typedef struct PMProperty_st
{
    pbool_t changed;
    pbool_t readonly;
    puint16_t appID;
    puint16_t serialID;
    char *name;
    PropertyType_t type;
    PropertyValue_t value;
    PLIST_ENTRY(struct PMProperty_st);
}PMProperty_t;

typedef enum
{
    SUBDEV_AUTH_NONE = 0,
    SUBDEV_AUTH_START,
    SUBDEV_AUTH_SUCCESS,
    SUBDEV_AUTH_FAILED,
}SubDevAuthStatus_t;

typedef struct ModulesVersion_st
{
    char *name;
    char version[PLATFORM_VERSION_LEN + 1];
    PLIST_ENTRY(struct ModulesVersion_st);
}ModulesVersion_t;

/*从设备信息*/
typedef struct PMSubDevice_st
{
    pbool_t online;
    pbool_t needPost;
    pbool_t onlineAcked;
    ptime_t onlineTime;
    ptime_t hbTime;
    SubDevAuthStatus_t authStatus;  //鉴权
    char did[PLATFORM_DEVID_LEN + 1];
    char pin[PLATFORM_PIN_LEN + 1];
    char model[PLATFORM_MODEL_LEN + 1];
    char version[PLATFORM_VERSION_LEN + 1];
    char factoryName[PLATFORM_FACTORY_NAME_LEN];
    int rssi;
    int battery;
    ModulesVersion_t modules;
    PMProperty_t property;
    ResourceInfo_t resource;
    PLIST_ENTRY(struct PMSubDevice_st);
}PMSubDevice_t;

struct PrivateCtx_st;
typedef struct
{
    pbool_t alloc;
    ptime_t lastMsgTime;
    PSocket_t *socket;
    struct PrivateCtx_st *ctx;
}PClientSocket_t;

typedef struct
{
    char *did;
    puint8_t type;
    char *name;
    char *version;
    puint8_t progress;

    //download
    puint8_t retriesCnt;
    pbool_t startDownload;
    char *url;
    HTTPRequest_t *request;
    puint32_t fileSize;
    puint32_t totalRecvLen;
}POtaInfo_t;

typedef struct PrivateCtx_st
{
    char did[PLATFORM_DEVID_LEN + 1];
    char pin[PLATFORM_PIN_LEN + 1];
    char model[PLATFORM_MODEL_LEN + 1];
    char factoryName[PLATFORM_FACTORY_NAME_LEN];

    char version[PLATFORM_VERSION_LEN + 1];
    char sessionkey[PLATFORM_SESSIONKEY_LEN + 1];
    char token[PLATFORM_TOKEN_LEN + 1];

    CMServerStatus_t serverStatus;
    ptime_t lastServerStatusTime;

    puint16_t hbIntervel;
    ptime_t lastHbSendTime;

    puint16_t sendsn;
    puint16_t recvsn;

    ModulesVersion_t modules;
    PMProperty_t properties;
    ResourceInfo_t resources;
    PMSubDevice_t subDevices; //从设备

    PlatformEventHandle_t eventHandle;

    PSocket_t *serverSocket;
    ptime_t lastServerMsgTime;

    PClientSocket_t appConnect[PLATFORM_CLIENT_CONNECT_NUM];

    POtaInfo_t ota;
}PrivateCtx_t;

//ptime_t PlatformTime(void);
#define PTimeDiff(new, old) ((new) - (old))
#define PTimeDiffCurrent(oldTime) PTimeDiff(PlatformTime(), (oldTime))
#define PTimeHasPast(oldTime, pastTime) (PTimeDiffCurrent((oldTime)) > (pastTime))

void PPrivateEventEmit(PrivateCtx_t *ctx, PlatformEvent_t event, void *args);
PMSubDevice_t *PPrivateGetSubDevice(PrivateCtx_t *ctx, const char *subDid);

int PPrivateSubDeviceHeartbeat(PrivateCtx_t *ctx, const char *did);
int PPrivateSubDeviceRSSIValue(PrivateCtx_t *ctx, const char *did, int rssi);
int PPrivateSubDeviceBatteryRemain(PrivateCtx_t *ctx, const char *did, int remain);

int PPrivateSubDeviceDel(PrivateCtx_t *ctx, const char *did);
int PPrivateSubDeviceRegister(PrivateCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName);
PrivateCtx_t *PPrivateCreate(void);
void PPrivateInitialize(void);
void PPrivatePoll(void);

#endif // !PPRIVATE_H
