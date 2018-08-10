#ifndef PLATFORM_H
#define PLATFORM_H

#define PROPERTY_INVALID_APPID 0xffff //��Ч��APPID

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
�豸״̬�澯��Ϣ
*/
typedef struct
{
    unsigned short serialId;
    PCommonInfo_t statusInfo;
    char *description;
    unsigned int time;
}PStatusAlarm_t;

/*
����������չ�ֶ�(��ѡ)
key/value ������
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
�������Ʋ���
*/
typedef struct
{
    char *format;             //��������ѹ����ʽ:"pcm","speex","amr"
    unsigned short rate;      //�������ݲ����ʣ�ȡֵ��8000,16000
    unsigned char channel;    //������������: 1��������
    char *voiceID;            //�������ݱ��,��¼�����ݷֶ�¼�Ʋ��ֶδ���ʱ���и�ֵ��һ��¼�����̸ñ����ͬ����ʽ��deviceId+ʱ���
    unsigned short sn;        //ÿ���ֶ�¼�����ݵ�˳��ţ��� 1 ��ʼ��������Ҫ�豸���շֶ�˳���ϴ��������ֶ��ϴ�ʱ��sequence ʹ��ͬһ��
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
    unsigned char type; //0�����豸����, 1����ģ������
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
    PEVENT_RESOURCE_ADD,          //�豸�����豸��Դ����
    PEVENT_RESOURCE_SET,          //�豸�����豸��Դ����
    PEVENT_RESOURCE_DEL,          //�豸�����豸��Դɾ��
    PEVENT_SUBDEV_AUTH_FAILED,    //���豸��Ȩʧ��
    PEVENT_SUBDEV_UNBINDED,       //���豸�����
    PEVENT_TIMING_INFO,           //��ʱ��Ϣ
    PEVENT_OTA_NOTICE,            //����֪ͨ
    PEVENT_OTA_DATA,              //��������
    PEVENT_OTA_FINISH,            //�������
    PEVENT_VOICE_CONTROL_RESULT,  //�������ƽ��
    PEVENT_TTS_RESULT,            //�����ϳɽ��
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
