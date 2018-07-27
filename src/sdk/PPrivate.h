#ifndef PPRIVATE_H
#define PPRIVATE_H

#include "PlatformCTypes.h"
#include "Adapter/PSocket.h"
#include "Util/PList.h"
#include "Platform.h"

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

typedef struct ResourceInfo_st
{
    char *name;
    puint16_t serialID;
    pbool_t *changed;
    puint16_t nodeNum;
    ResourceNode_t *node;
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
    SubDevAuthStatus_t authStatus;  //鉴权
    char did[PLATFORM_DEVID_LEN + 1];
    char pin[PLATFORM_PIN_LEN + 1];
    char model[PLATFORM_MODEL_LEN + 1];
    char version[PLATFORM_VERSION_LEN + 1];
    ModulesVersion_t modules;
    PMProperty_t property;
    ResourceInfo_t resource;
    PLIST_ENTRY(struct PMSubDevice_st);
}PMSubDevice_t;

typedef struct PrivateCtx_st
{
    char did[PLATFORM_DEVID_LEN + 1];
    char pin[PLATFORM_PIN_LEN + 1];
    char model[PLATFORM_MODEL_LEN + 1];

    char version[PLATFORM_VERSION_LEN + 1];
    char sessionkey[PLATFORM_SESSIONKEY_LEN + 1];
    char token[PLATFORM_TOKEN_LEN + 1];

    CMServerStatus_t serverStatus;
    ptime_t lastServerStatusTime;
    puint16_t hbIntervel;
    ptime_t lastHbTime;

    puint16_t sendsn;
    puint16_t recvsn;

    ModulesVersion_t modules;
    PMProperty_t properties;
    ResourceInfo_t resources;
    PMSubDevice_t subDevices; //从设备

    PlatformEventHandle_t eventHandle;
    PSocket_t *serverSocket;
    PSocket_t clientSockets[PLATFORM_CLIENT_CONNECT_NUM];
}PrivateCtx_t;

//ptime_t PlatformTime(void);
#define PTimeDiff(new, old) ((new) - (old))
#define PTimeDiffCurrent(oldTime) PTimeDiff(PlatformTime(), (oldTime))
#define PTimeHasPast(oldTime, pastTime) (PTimeDiffCurrent((oldTime)) > (pastTime))

void PPrivateEventEmit(PrivateCtx_t *ctx, PlatformEvent_t event, void *args);
int PPrivateSubDeviceDel(PrivateCtx_t *ctx, const char *did);
int PPrivateSubDeviceRegister(PrivateCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version);
PrivateCtx_t *PPrivateCreate(void);
void PPrivateInitialize(void);
void PPrivatePoll(void);

#endif // !PPRIVATE_H
