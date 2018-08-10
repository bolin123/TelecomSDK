#include "UserTypes.h"
#include "Platform.h"
#include "PlatformMc.h"
#include <time.h>

#ifdef __WIN__
#include <Windows.h>
#endif

static PlatformCtx_t *g_ctx;
static char *g_devID = "00B00100FF01001001000150294D20F648";
static char *g_devPin = "50294D20F64850294D20F64850294D20";

static char *g_subDevID = "00B0010003FF0111033856EB04004B1200";
static char *g_subDevPin = "3856EB04004B12003856EB04004B1200";
static bool g_alarmEnable = true;

/*
bool UserPhyIsLinkup(void)
{
    return true;
}

uint32_t PlatformUserUtcTime(void)
{
    return (uint32_t)time(NULL);
}

uint32_t UserGetTickCount(void)
{
    return (uint32_t)GetTickCount();
}
*/

static void propertyContrl(PPropertyCtrl_t *pctrl)
{
    int i;
    if(pctrl)
    {
        for(i = 0; i < pctrl->propertyNum; i++)
        {
            /*subdeivce or master*/
            if(strcmp(pctrl->did, g_subDevID) == 0)
            {
                //control subdevice
                if(pctrl->property[i].appid == 1)
                {
                    g_alarmEnable = pctrl->property[i].value.num != 0;
                }
            }
            else
            {
                //master
            }

            if(pctrl->property[i].isText) //after operate,upgrade property value
            {
                PlatformPropertySetTextValue(g_ctx, pctrl->did, pctrl->property[i].appid, pctrl->property[i].value.text);
            }
            else
            {
                PlatformPropertySetNumValue(g_ctx, pctrl->did, pctrl->property[i].appid, pctrl->property[i].value.num);
            }
        }
    }
}

static void platformEventHandle(PlatformEvent_t event, void *args)
{
    int value;
    bool success;
    PResources_t *resource = (PResources_t *)args;

    switch(event)
    {
    case PEVENT_SERVER_ON_OFFLINE:     //�������ϡ�����״̬
        ulog("PEVENT_SERVER_ON_OFFLINE %d", (int)args);
        break;
    case PEVENT_PROPERTY_CONTRL:       //�豸�����豸״̬����
        propertyContrl((PPropertyCtrl_t *)args);
        break;
    case PEVENT_RESOURCE_ADD:          //�豸�����豸��Դ����
        //todo: add resource
        PlatformResourceItemAdd(g_ctx, resource->did, resource->rscName, resource->rscID, resource->info, resource->infoNum);
        break;
    case PEVENT_RESOURCE_SET:          //�豸�����豸��Դ����
        //todo:set resource
        PlatformResourceItemSet(g_ctx, resource->did, resource->rscName, resource->rscID, resource->info, resource->infoNum);
        break;
    case PEVENT_RESOURCE_DEL:          //�豸�����豸��Դɾ��
        //todo:del resource
        PlatformResourceItemDel(g_ctx, resource->did, resource->rscName, resource->rscID);
        break;
    case PEVENT_SUBDEV_AUTH_FAILED:    //���豸��Ȩʧ��
        ulog("subDevice %s PEVENT_SUBDEV_AUTH_FAILED!", (char *)args);
        break;
    case PEVENT_SUBDEV_UNBINDED:       //���豸�����
        ulog("subDevice %s PEVENT_SUBDEV_UNBINDED!", (char *)args);
        break;
    case PEVENT_TIMING_INFO:           //��ʱ��Ϣ
        break;
    case PEVENT_OTA_NOTICE:            //����֪ͨ
        break;
    case PEVENT_OTA_DATA:              //��������
        break;
    case PEVENT_OTA_FINISH:            //����״̬

        break;
    case PEVENT_VOICE_CONTROL_RESULT:  //�������ƽ��
        break;
    case PEVENT_TTS_RESULT:
        break;
    default:
        break;
    }
}


/*���豸ע��(�Ŵ�)

*/

static void subDeviceRegist(void)
{
    char *model = "doorSensor";
    char *version = "111.222.333.444";
    PlatformSubDeviceRegister(g_ctx, g_subDevID, g_subDevPin, model, version);

    PPropertyInfo_t pInfo;
    pInfo.appid = 1;
    pInfo.sid = 0;
    pInfo.name = "ALARM";
    pInfo.isText = false;
    pInfo.readonly = false;
    PlatformPropertyRegister(g_ctx, g_subDevID, &pInfo);

    pInfo.appid = 2;
    pInfo.name = "DOOR_OPEN";
    pInfo.readonly = true;
    PlatformPropertyRegister(g_ctx, g_subDevID, &pInfo);

    PlatformPropertySetNumValue(g_ctx, g_subDevID, 1, g_alarmEnable); //�����澯
    PlatformPropertySetNumValue(g_ctx, g_subDevID, 2, 0); //����

    PlatformSubDeviceOnOffline(g_ctx, g_subDevID, true);//����
}

static void propertiesConfig(void)
{
    PPropertyInfo_t pInfo;

    pInfo.appid = 1;
    pInfo.sid = 0;
    pInfo.name = "LIGHT_TYPE";
    pInfo.isText = false;
    pInfo.readonly = false;
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid = 2;
    pInfo.name = "LIGHT_MODE";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid = 3;
    pInfo.name = "SND_TYPE";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid = 4;
    pInfo.readonly = true;
    pInfo.name = "VOLUME";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    PlatformPropertySetNumValue(g_ctx, g_devID, 1, 1); //�����
    PlatformPropertySetNumValue(g_ctx, g_devID, 2, 0); //�رյƹ�
    PlatformPropertySetNumValue(g_ctx, g_devID, 3, 0); //����������
    PlatformPropertySetNumValue(g_ctx, g_devID, 4, 55); //������С
}

static uint32_t g_lastTime;
static void testSubDeviceReport(void)
{
    static bool doorOpened = false;
    if(g_alarmEnable && (PlatformTime() - g_lastTime) > 30000)
    {
        PlatformPropertySetNumValue(g_ctx, g_subDevID, 2, doorOpened); //����
        doorOpened = !doorOpened;
        g_lastTime = PlatformTime();
    }
}

void miniGWInit(void)
{

    PlatformInitialize();
    g_ctx = PlatformCtxCreate();
    if(g_ctx)
    {
        PlatformEventRegister(g_ctx, platformEventHandle);
        PlatformSetDeviceInfo(g_ctx, g_devID, g_devPin, "1001", "001.000.000.001");
        propertiesConfig();

        subDeviceRegist();

        PlatformStart(g_ctx);
    }
    
}

void miniGWPoll(void)
{

    PlatformPoll(g_ctx);
    testSubDeviceReport();
}

