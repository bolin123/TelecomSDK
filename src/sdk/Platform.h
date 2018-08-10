#ifndef PLATFORM_H
#define PLATFORM_H

#define PROPERTY_INVALID_APPID 0xffff //无效的APPID

typedef unsigned char ptBool_t;

typedef struct
{
    ptBool_t isText;
    char *name;
    union
    {
        int num;
        char *text;
    }value;
}PCommonInfo_t;

typedef struct
{
    unsigned char *data;
    unsigned int datalen;
    unsigned int fileSize;
}POTADataArgs_t;

typedef struct
{
    unsigned short serialId;
    int errCode;
    char *errInfo;
    unsigned int time;
}PErrorReport_t;

/*
设备状态告警信息
*/
typedef struct
{
    unsigned short serialId;
    PCommonInfo_t statusInfo;
    char *description;
    unsigned int time;
}PStatusAlarm_t;

/*
语音控制扩展字段(可选)
key/value 数据组
*/
typedef struct
{
    char *key;
    char *value;
}PVoiceCtrlExtension_t;

typedef struct
{
    char *did;
    char *rawText;
    char *description;
}PVoiceCtrlResult_t;

typedef struct
{
    char *did;
    char *content;
}PTTSResult_t;

typedef struct
{
    char *content;
    char *anchor;
    unsigned char speed;
    unsigned char volume;
    unsigned char pitch;
    char *format;
    unsigned short rate;
    unsigned char channel;
}PTTSParameter_t;

/*
语音控制参数
*/
typedef struct
{
    char *format;             //语音数据压缩格式:"pcm","speex","amr"
    unsigned short rate;      //语音数据采样率，取值：8000,16000
    unsigned char channel;    //语音数据声道: 1：单声道
    char *voiceID;            //语音数据编号,在录音数据分段录制并分段传输时进行赋值，一次录音过程该编号相同，格式：deviceId+时间戳
    unsigned short sn;        //每个分段录音数据的顺序号，从 1 开始递增，需要设备按照分段顺序上传，语音分段上传时，sequence 使用同一个
}PVoiceCtrlParameter_t;

typedef struct
{
    char *name;
    unsigned short serialID;
    unsigned char infoNum;
    PCommonInfo_t *info;
}PEventInfo_t;

typedef struct
{
    unsigned char type; //0：给设备升级, 1：给模块升级
    char *name;
    char *url;
    char *version;
    char *md5;
}PUpgradeInfo_t;

typedef struct
{
    char *did;
    int infoNum;
    PUpgradeInfo_t *info;
}PUpgradeNotice_t;

typedef struct
{
    unsigned short sid;
    char *did;
    char *rscName;
    unsigned short rscID;
    int infoNum;
    PCommonInfo_t *info;
}PResources_t;

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
    unsigned short sid;
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
    PEVENT_RESOURCE_ADD,          //设备、子设备资源新增
    PEVENT_RESOURCE_SET,          //设备、子设备资源设置
    PEVENT_RESOURCE_DEL,          //设备、子设备资源删除
    PEVENT_SUBDEV_AUTH_FAILED,    //子设备鉴权失败
    PEVENT_SUBDEV_UNBINDED,       //子设备被解绑
    PEVENT_TIMING_INFO,           //授时信息
    PEVENT_OTA_NOTICE,            //升级通知
    PEVENT_OTA_DATA,              //升级数据
    PEVENT_OTA_FINISH,            //升级完成
    PEVENT_VOICE_CONTROL_RESULT,  //语音控制结果
    PEVENT_TTS_RESULT,            //语音合成结果
}PlatformEvent_t;

typedef void (*PlatformEventHandle_t)(PlatformEvent_t event, void *args);

void PlatformTTS(PlatformCtx_t *ctx, const char *did, PTTSParameter_t *param);
void PlatformErrorReport(PlatformCtx_t *ctx, const char *did, PErrorReport_t *error);
void PlatformStatusAlarm(PlatformCtx_t *ctx, const char *did, PStatusAlarm_t *alarm);
void PlatformVoiceControl(PlatformCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum);
void PlatformEventInfoReport(PlatformCtx_t *ctx, const char *did, PEventInfo_t *event);

int PlatformResourceItemSet(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);
int PlatformResourceItemDel(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id);
int PlatformResourceItemAdd(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);
int PlatformResourceKeywordRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *keyword, ptBool_t valueIsText);
int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *idName, unsigned short serialId);

int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value);
int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value);
int PlatformPropertyRegister(PlatformCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo);

void PlatformStartOTA(PlatformCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo);
void PlatformOTAResult(PlatformCtx_t *ctx, ptBool_t success);

void PlatformSetModuleVersion(PlatformCtx_t *ctx, const char *did, const char *name, const char *version);
void PlatformSetDeviceInfo(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version);
void PlatformStart(PlatformCtx_t *ctx);

int PlatformSubDeviceUnbind(PlatformCtx_t *ctx, const char *did);
int PlatformSubDeviceOnOffline(PlatformCtx_t *ctx, const char *did, ptBool_t online);
int PlatformSubDeviceRegister(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version);

ptBool_t PlatformServerConnected(PlatformCtx_t *ctx);
void PlatformEventRegister(PlatformCtx_t *ctx, PlatformEventHandle_t handle);

PlatformCtx_t *PlatformCtxCreate(void);
void PlatformInitialize(void);
void PlatformPoll(PlatformCtx_t *ctx);

#endif // !PLATFORM_H
