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
    char *name;           //��������
    ptBool_t isText;      //�Ƿ�Ϊ�ı�����
    ptBool_t readonly;    //�Ƿ�Ϊֻ��
    unsigned short appid; //app��Ӧ������ID
    unsigned short sid;   //��·�豸��·����0 ��ʾΪ��·�豸��1��2��3 ��ʾ��·�豸�ĵ� 1��2��3 ·
}PPropertyInfo_t;

struct PrivateCtx_st;
typedef struct PlatformCtx_st
{
    struct PrivateCtx_st *private;
}PlatformCtx_t;

typedef enum
{
    PEVENT_SERVER_ON_OFFLINE,     //�������ϡ�����״̬
    PEVENT_PROPERTY_CONTRL,       //�豸�����豸״̬����
    PEVENT_RESOURCE_CONTRL,       //�豸�����豸��Դ����
    PEVENT_SUBDEV_AUTH_FAILED,    //���豸��Ȩʧ��
    PEVENT_SUBDEV_UNBINDED,       //���豸�����
    PEVENT_TIMING_INFO,           //��ʱ��Ϣ
    PEVENT_OTA_NOTICE,            //����֪ͨ
    PEVENT_OTA_DATA,              //��������
    PEVENT_OTA_STATUS,            //����״̬
    PEVENT_VOICE_CONTROL_RESULT,  //�������ƽ��
    PEVENT_TTS_RESULT,            //�����ϳɽ��
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
