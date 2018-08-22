/*
 * Platform.h
 *
 * SDK�ӿ��ļ���������ȫ���ṹ�弰���󲿷ֽӿ�
 * �û�ʹ��SDK��Ҫ�������ļ�
 *
 * By Berlin 2018.08 <chenbl@yumair.cn>
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#define PROPERTY_INVALID_APPID 0xffff //��Ч��APPID

typedef unsigned char ptBool_t;

/*
* ����������Ϣ
*/
typedef struct
{
    ptBool_t isText; //�Ƿ�Ϊ�ı�
    char *name;
    union
    {
        int num;
        char *text;
    }value;
}PCommonInfo_t;

/*
* �����̼�����
*/
typedef struct
{
    unsigned char *data;   //��ǰ��������
    unsigned int datalen;  //��ǰ���ݴ�С
    unsigned int fileSize; //�ܵ��ļ���С
}POTADataArgs_t;

/*
* �����ϱ�
*/
typedef struct
{
    unsigned short serialId; //��·�豸��·����0��ʾ��·�豸
    int errCode;             //������
    char *errInfo;           //������Ϣ
    unsigned int time;       //���Ϸ���utcʱ��
}PErrorReport_t;

/*
* ״̬�澯��Ϣ
*/
typedef struct
{
    unsigned short serialId;  //��·�豸��·����0��ʾ��·�豸
    PCommonInfo_t statusInfo; //�澯���豸������Ϣ
    char *description;        //����
    unsigned int time;        //�澯����utcʱ��
}PStatusAlarm_t;

/*
* ����������չ�ֶ�(��ѡ)
* key/value ������
*/
typedef struct
{
    char *key;
    char *value;
}PVoiceCtrlExtension_t;

/*
* �������ƽ��
*/
typedef struct
{
    char *did;  //��/���豸ID
    char *rawText; //����ʶ��ת���ɵ��ı�
    char *description; //�������
}PVoiceCtrlResult_t;

/*
* �����ϳɽ��
*/
typedef struct
{
    char *did; //��/���豸ID
    char *content; //�ϳɵ���Ƶ����BASE64����
}PTTSResult_t;

/*
* �����ϳɲ���
*/
typedef struct
{
    char *content;        //�����ϳ��ı�����
    char *anchor;         //�����˱�ʶ(������ΪĬ�����ķ�����)
    unsigned char speed;  //����(0~100)
    unsigned char volume; //����(0~100)
    unsigned char pitch;  //����(0~100)
    char *format;         //���������ʽ(pcm\speex\amr)
    unsigned short rate;  //������(8000\16000)
    unsigned char channel; //����(1:������)
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

/*
* ��Ϣ/�¼�
*/
typedef struct
{
    char *name; //����
    unsigned short serialID; //��·�豸��·����0��ʾ��·�豸
    unsigned char infoNum; //��Ϣ����
    PCommonInfo_t *info;   //��Ϣ����
}PEventInfo_t;

/*
* ������Ϣ
*/
typedef struct
{
    unsigned char type; //0�����豸����, 1����ģ������
    char *name; //�̼����ƣ������豸Ϊ��
    char *url;  //���ӵ�ַ
    char *version; //�汾��
    char *md5;  //MD5У��ֵ
}PUpgradeInfo_t;

/*
* ����֪ͨ
*/
typedef struct
{
    char *did;   //�豸ID
    int infoNum; //��Ϣ����(�п���ͬʱ�ж���̼���Ϣ���豸����ѡ��һ��)
    PUpgradeInfo_t *info; //��Ϣ����
}PUpgradeNotice_t;

/*
* ��Դ
*/
typedef struct
{
    unsigned short sid; //��·�豸��·����0��ʾ��·�豸
    char *did;     //�豸ID
    char *rscName; //��Դ��
    unsigned short rscID; //��ԴID
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

/*
* �豸״̬����
*/
typedef struct
{
    char *did; //�豸ID
    unsigned short sid; //��·�豸��·����0��ʾ��·�豸
    int propertyNum; //���Ը���
    PPropertyValue_t *property; //����
}PPropertyCtrl_t;

typedef struct PPropertyInfo_st
{
    char *name;           //��������
    ptBool_t isText;      //�Ƿ�Ϊ�ı�����
    ptBool_t readonly;    //�Ƿ�Ϊֻ��
    unsigned short appid; //app��Ӧ������ID���û��Լ����壬ÿ������appidΨһ��
    unsigned short sid;   //��·�豸��·����0 ��ʾΪ��·�豸��1��2��3 ��ʾ��·�豸�ĵ� 1��2��3 ·
}PPropertyInfo_t;

/*ƽ̨���ṹ��*/
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

/*�¼��ص�����*/
typedef void(*PlatformEventHandle_t)(PlatformEvent_t event, void *args);

/*
* �����ϳ�
* ͨ���ýӿ������ܼҾ�ƽ̨���������ϳ����������ı���ȡ����Ӧ��������
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @param:�����ϳɲ���
*/
void PlatformTTS(PlatformCtx_t *ctx, const char *did, PTTSParameter_t *param);

/*
* ��������
* �豸�ϴ����������ݣ�ƽ̨��������ʶ�𲢵õ��豸���Ƶ�ָ�֧�ֶַν����������ݣ�
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @param:��������
*      @data:��������
*      @isLast:�Ƿ�Ϊ���һ�����ݣ�1�������һ�����ݣ�0���������һ������
*      @ext:��չ�ֶΣ���ѡ����ʹ��ΪNULL��
*      @extNum:��չ�ֶνṹ���������ʹ����չ�ֶ�Ϊ0��
*/
void PlatformVoiceControl(PlatformCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum);

/*
* �豸���Ϸ���
* ���豸�������豸��������ʱ���豸��ͨ�����ӿ���Ϣ���͹��ϴ����б�����ܼҾ�ƽ̨
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @error:������Ϣ
*/
void PlatformErrorReport(PlatformCtx_t *ctx, const char *did, PErrorReport_t *error);

/*
* ״̬�澯
* �豸��⵽�豸���Գ����趨�ķ�ֵʱ��ͨ���ýӿڷ��͸澯���ݵ����ܼҾ�ƽ̨
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @alarm:�澯��Ϣ
*/
void PlatformStatusAlarm(PlatformCtx_t *ctx, const char *did, PStatusAlarm_t *alarm);

/*
* �豸��Ϣ�ϱ�
* �豸���й����в�������Ϣ/�¼��������塢���ţ���ͨ���ýӿ��ϱ�
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @alarm:��Ϣ����
*/
void PlatformEventInfoReport(PlatformCtx_t *ctx, const char *did, PEventInfo_t *event);

/*------------------------------�豸��Դ--------------------------------*/

/*
* �豸��Դ�޸�
* �豸�����޸�����Դ���ݣ�ͨ���ýӿ��ϱ���ƽ̨
* ����: @ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @rscName:��Դ��������
*      @id:��ԴID
*      @value:�޸�����
*      @valNum:���ݵĸ���
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformResourceItemSet(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);

/*
* �豸��Դɾ��
* �豸����ɾ������Դ���ݣ�ͨ���ýӿ��ϱ���ƽ̨
* ����: @ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @rscName:��Դ��������
*      @id:��ԴID
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformResourceItemDel(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id);

/*
* �豸��Դ����
* �豸�����������µ���Դ��ͨ���ýӿ��ϱ���ƽ̨
* ����: @ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @rscName:��Դ��������
*      @id:��ԴID
*      @value:��Դ����
*      @valNum:���ݵĸ���
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformResourceItemAdd(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);

/*
* ע����Դ���ƹؼ���
* ��Ҫ����ͨ���ýӿ�ע���ID����֮�����Դ����
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @rscName:��Դ��������
*      @keyword:��Դ�ؼ�������
*      @valueIsText:����Դ��ֵ�Ƿ�Ϊ�ַ���true���ַ���false����ֵ
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformResourceKeywordRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *keyword, ptBool_t valueIsText);

/*
* ��Դע��
* �κβ�����Դ֮ǰ��Ҫ�������Դ����
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @rscName:��Դ��������
*      @idName:��ԴID����
*      @serialId:��·�豸��·����0��ʾ��·�豸
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, const char *idName, unsigned short serialId);

/*------------------------------�豸״̬(����)--------------------------------------*/

/*
* �����豸״̬(����)��ֵ���ַ����ͣ�
* �豸�������״̬�ı����Ҫͨ���ýӿ��ϱ���ƽ̨״̬�仯
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @appid:��״̬��APP�����IDֵ���û��Լ����壩
*      @value:״ֵ̬
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformPropertySetTextValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, const char *value);

/*
* �����豸״̬(����)��ֵ����ֵ���ͣ�
* �豸�������״̬�ı����Ҫͨ���ýӿ��ϱ���ƽ̨״̬�仯
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @appid:��״̬��APP�����IDֵ���û��Լ����壩
*      @value:״ֵ̬
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformPropertySetNumValue(PlatformCtx_t *ctx, const char *did, unsigned short appid, unsigned int value);

/*
* ע���豸״̬(����)
* �����豸״̬(����)֮ǰ��Ҫ�Ƚ���ע��
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @pInfo:�豸״̬(����)��Ϣ
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformPropertyRegister(PlatformCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo);

/*
* ��ʼ�̼�����
* ƽ̨���°�̼�ʱ�����¼�����ʽ֪ͨ�豸���豸ѡ��ĳһ���̼���������
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @upgradeInfo:�̼���Ϣ
*/
void PlatformStartOTA(PlatformCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo);

/*
* ��������ϱ�
* �豸������ɺ��ϱ��������
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @success:���������true���ɹ���false��ʧ��
*/
void PlatformOTAResult(PlatformCtx_t *ctx, ptBool_t success);

/*
* �����豸ģ��汾��
* ����ƽ̨�Ա��Ƿ����°汾�Ĺ̼����Ƿ���Ҫ����������Ϣ
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:��/���豸ID
*      @name:ģ������
*      @version���汾��
*/
void PlatformSetModuleVersion(PlatformCtx_t *ctx, const char *did, const char *name, const char *version);

/*
* �����豸��Ϣ
* ���������豸����Ҫ��Ϣ������ƽ̨��¼�ͼ�Ȩ
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:�豸ID,�豸�ڵ������ܼҾ�ƽ̨��Ψһ��ʶ
*      @pin:�豸PIN��
*      @model:�豸�ͺţ��ں����ұ�ʶ��Ʒ�Ʊ�ʶ���ͺű�ʶ��
*      @version:Э��汾���汾�Ÿ�ʽΪ XXX.YYY.ZZZ.MMM
*      @factoryName:��������
*/
void PlatformSetDeviceInfo(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName);

/*
* ��ʼִ��ƽ̨����ҵ��
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*/
void PlatformStart(PlatformCtx_t *ctx);

/*-----------------------------���豸����-------------------------------------*/

/*
* ���豸����
* �����յ����豸����������ʱ��ͨ���ýӿڸ�������ʱ��
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceHeartbeat(PlatformCtx_t *ctx, const char *did);

/*
* ���豸�����ź�ǿ�ȣ���ѡ��
* ���´��豸���ź�ǿ�ȣ�����ʱ�����豸���Լ����壩
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
*      @rssi:�ź�ǿ��
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceRSSIValue(PlatformCtx_t *ctx, const char *did, int rssi);

/*
* ���豸��ص�������ѡ��
* ���´��豸�ĵ���ʣ��ֵ������ʱ�����豸���Լ����壩
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
*      @remain:ʣ�����
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceBatteryRemain(PlatformCtx_t *ctx, const char *did, int remain);

/*
* ���豸����ϱ�
* �����ĳ�����豸����Ҫͨ���ýӿ��ϱ���ƽ̨
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceUnbind(PlatformCtx_t *ctx, const char *did);

/*
* ���豸�ϡ�����
* ͨ���ýӿ��ϱ�ƽ̨���豸�ϡ�����֪ͨ
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
*      @online:���߱�־��true�����ߣ�false������
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceOnOffline(PlatformCtx_t *ctx, const char *did, ptBool_t online);

/*
* ���豸��Ϣע��
* ���豸����֮ǰ��Ҫ�Ƚ�����Ϣע��
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @did:���豸ID
*      @pin:���豸PIN��
*      @model:�豸�ͺţ��ں����ұ�ʶ��Ʒ�Ʊ�ʶ���ͺű�ʶ��
*      @version:Э��汾���汾�Ÿ�ʽΪ XXX.YYY.ZZZ.MMM
*      @factoryName:��������
* ����ֵ:0:�ɹ���< 0:ʧ��
*/
int PlatformSubDeviceRegister(PlatformCtx_t *ctx, const char *did, const char *pin, const char *model, const char *version, const char *factoryName);

/*
* �Ƿ����ӵ��������ܼҾ�ƽ̨
* ������@ctx:���ṹ��(PlatformCtxCreate����)
* ����ֵ:true�������ӣ�false��δ����
*/
ptBool_t PlatformServerConnected(PlatformCtx_t *ctx);

/*
* ƽ̨�¼��ص�����ע��
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*      @handle:�ص�����
*/
void PlatformEventRegister(PlatformCtx_t *ctx, PlatformEventHandle_t handle);

/*
* ����ƽ̨�������ṹ��
* ����ֵ:!=NULL�����ṹ�壬NULL������ʧ��
*/
PlatformCtx_t *PlatformCtxCreate(void);

/*
* ƽ̨��ʼ������
*/
void PlatformInitialize(void);

/*
* ƽ̨��ѭ������
* ������@ctx:���ṹ��(PlatformCtxCreate����)
*/
void PlatformPoll(PlatformCtx_t *ctx);

#endif // !PLATFORM_H
