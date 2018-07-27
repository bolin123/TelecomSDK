#ifndef PLATFORM_H
#define PLATFORM_H

#define PROPERTY_INVALID_APPID 0xffff

typedef unsigned char ptBool_t;

typedef struct
{
    char *name;
    char *url;
    char *version;
    char *md5;
}PUgradeInfo_t;

typedef struct
{
    int infoNum;
    PUgradeInfo_t *info;
}PUpgradeNotice_t;

typedef struct
{
    ptBool_t isText;
    char *name;
    union
    {
        int num;
        char *text;
    }value;
}PResourceInfo_t;

typedef struct
{
    unsigned short serialID;
    char *resourceName;
    int infoNum;
    PResourceInfo_t *info;
}PResources_t;

typedef struct
{
    int resourceNum;
    PResources_t *resource;
}PResourceManager_t;

typedef struct
{
    ptBool_t isText;
    unsigned short appid;
    union
    {
        int num;
        char *text;
    }value;
}PPropertyValue_t;

typedef struct
{
    char *did;
    int propertyNum;
    PPropertyValue_t *property;
}PPropertyCtrl_t;

typedef struct PPropertyInfo_st
{
    char *name;           //属性名称
    ptBool_t isText;      //是否为文本属性
    ptBool_t readonly;    //是否为只读
    unsigned short appid; //app对应的属性ID
    unsigned short sid;   //多路设备的路数，0 表示为单路设备，1、2、3 表示多路设备的第 1、2、3 路
}PPropertyInfo_t;

struct PrivateCtx_st;
typedef struct PlatformCtx_st
{
    struct PrivateCtx_st *private;
}PlatformCtx_t;

typedef enum
{
    PEVENT_SERVER_ON_OFFLINE,     //服务器上、下线状态
    PEVENT_PROPERTY_CONTRL,       //设备、子设备状态控制
    PEVENT_RESOURCE_CONTRL,       //设备、子设备资源设置
    PEVENT_SUBDEV_AUTH_FAILED,    //子设备鉴权失败
    PEVENT_SUBDEV_UNBINDED,       //子设备被解绑
    PEVENT_TIMING_INFO,           //授时信息
    PEVENT_OTA_NOTICE,            //升级通知
    PEVENT_OTA_DATA,              //升级数据
    PEVENT_OTA_STATUS,            //升级状态
    PEVENT_VOICE_CONTROL_RESULT,  //语音控制结果
    PEVENT_TTS_RESULT,            //语音合成结果
}PlatformEvent_t;

typedef void (*PlatformEventHandle_t)(PlatformEvent_t event, void *args);

int PlatformResourceInfoSetNumValue(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, int value);
int PlatformResourceInfoSetTextValue(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, const char *value);
int PlatformResourceInfoRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, ptBool_t isText);
int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *name, unsigned short sid, unsigned short infoNum);

int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value);
int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value);
int PlatformPropertyRegister(PlatformCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo);

void PlatformSetModuleVersion(PlatformCtx_t *ctx, const char *did, const char *name, const char *version);
void PlatformSetDeviceInfo(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version);
void PlatformStart(PlatformCtx_t *ctx);

int PlatformSubDeviceUnbind(PlatformCtx_t *ctx, const char *did);
int PlatformSubDeviceOnOffline(PlatformCtx_t *ctx, const char *did, ptBool_t online);
int PlatformSubDeviceRegister(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version);

ptBool_t PlatformServerConnected(PlatformCtx_t *ctx);
void PlatformEventRegister(PlatformEventHandle_t handle);

PlatformCtx_t *PlatformCtxCreate(void);
void PlatformInitialize(void);
void PlatformPoll(PlatformCtx_t *ctx);

#endif // !PLATFORM_H
