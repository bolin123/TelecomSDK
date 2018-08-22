#include "demo1.h"
#if defined(COMPILE_DEMO_1)
#include "Adapter/PAdapter.h"
#include "Platform.h"

#ifdef __WIN__
#include <Windows.h>
#endif

/*
deviceId:00B0170001030510021891284285528661pin:30391774148976657263235920075581

*/
static PlatformCtx_t *g_ctx;
static char *g_devID = "00B0170001030510021891284285528661";//"00B0170001030510022018332626142236";//"00B0070103010010014293641078460438";
static char *g_devPin = "30391774148976657263235920075581";//"43258429108334654789321457832222";//"18596809735815542467406343584693";

ROM_FUNC static void testTTS(void)
{
    PTTSParameter_t param;

    param.content = "air purifier opened";
    param.channel = 1;
    param.format = "amr";
    param.pitch  = 50;
    param.rate   = 1600;
    param.speed  = 50;
    param.volume = 50;
    param.anchor = NULL;

    PlatformTTS(g_ctx, g_devID, &param);
}

ROM_FUNC static void testEventReport(void)
{
    PEventInfo_t event;

    event.serialID = 0;
    event.name = "LOCK_OPEN";
    event.infoNum = 2;
    event.info = (PCommonInfo_t *)malloc(sizeof(PCommonInfo_t) * event.infoNum);
    if(event.info)
    {
        event.info[0].isText = false;
        event.info[0].name = "SAFE_LOCK";
        event.info[0].value.num = 0;

        event.info[1].isText = false;
        event.info[1].name = "TIME";
        event.info[1].value.num = PUtcTime();
        PlatformEventInfoReport(g_ctx, g_devID, &event);
        free(event.info);
    }
}

ROM_FUNC static void testStatusAlarm(void)
{
    PStatusAlarm_t alarm;

    alarm.serialId = 0;
    alarm.statusInfo.isText = false;
    alarm.statusInfo.name = "FAN_SPEED";
    alarm.statusInfo.value.num = 12;
    alarm.description = "���ٹ���";
    alarm.time = PUtcTime();
    PlatformStatusAlarm(g_ctx, g_devID, &alarm);
}

ROM_FUNC static void testErrorReport(void)
{
    PErrorReport_t error;

    error.serialId = 0;
    error.errCode = 100001;
    error.errInfo = "������Ϣ:XXXXX!";
    error.time = PUtcTime();
    PlatformErrorReport(g_ctx, g_devID, &error);
}

ROM_FUNC static void propertyContrl(PPropertyCtrl_t *pctrl)
{
    int i;
    if(pctrl)
    {
        for(i = 0; i < pctrl->propertyNum; i++)
        {
            //todo: device contrl...
            if(pctrl->property[i].appid == 2)//FAN_SPEED
            {
                if(pctrl->property[i].value.num > 3)
                {
                    PlatformPropertySetNumValue(g_ctx, pctrl->did, 5, 0); //WORK_MODE,�ر��Զ�ģʽ
                }
            }

            if(pctrl->property[i].appid == 10)
            {
                //testTTS();//����tts
                testEventReport();
                testStatusAlarm();
                testErrorReport();
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

ROM_FUNC static void timing(uint32_t utctime)
{
    ulog("utc:%d", utctime);
    //todo timing...
}

ROM_FUNC static void ttsResultHandle(PTTSResult_t *tts)
{
    ulog("did:%s, content:%s", tts->did, tts->content);
}

ROM_FUNC static void otaNoticeHandle(PUpgradeNotice_t *notice)
{
    int i;
    PUpgradeInfo_t *info = NULL;

    if(notice)
    {
        ulog("upgrade device id:%s", notice->did);

        // ÿ��ֻ��ѡ���豸��һ��ģ��̼���������
        for(i = 0; i < notice->infoNum; i++)
        {
            if(notice->info[i].name == NULL) //�豸������Ϣ
            {
                ulog("device OTA version %s", notice->info[i].version);
                info = &notice->info[i];
            }
            else  //ģ��������Ϣ
            {
                ulog("module %s OTA version %s", notice->info[i].name, notice->info[i].version);
                info = &notice->info[i];
                break; //������������ģ��(����ʵ����������ѡ��)
            }
        }

        if(info)
        {
            PlatformStartOTA(g_ctx, notice->did, info);
        }
    }
}

ROM_FUNC static void otaDataHandle(POTADataArgs_t *args)
{
    if(args)
    {
        ulog("new data, size %d", args->datalen);
        //TODO: save data...
    }
}

ROM_FUNC static void platformEventHandle(PlatformEvent_t event, void *args)
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
            timing((uint32_t)args);
            break;
        case PEVENT_OTA_NOTICE:            //����֪ͨ
            otaNoticeHandle((PUpgradeNotice_t *)args);
            break;
        case PEVENT_OTA_DATA:              //��������
            otaDataHandle((POTADataArgs_t *)args);
            break;
        case PEVENT_OTA_FINISH:            //����״̬
            success = (bool)args;
            if(success)
            {
                //if(checkFirmwareOk())
                PlatformOTAResult(g_ctx, true);
                //else
                //PlatformOTAResult(g_ctx, false);
            }
            else
            {
                PlatformOTAResult(g_ctx, false);

            }
            break;
        case PEVENT_VOICE_CONTROL_RESULT:  //�������ƽ��
            break;
        case PEVENT_TTS_RESULT:
            ttsResultHandle((PTTSResult_t *)args);
            break;
        default:
            break;
    }
}

ROM_FUNC static void propertiesConfig(void)
{
    PPropertyInfo_t pInfo;

    pInfo.appid    = 1;
    pInfo.sid      = 0;
    pInfo.name     = "POWER";
    pInfo.isText   = false;
    pInfo.readonly = false;
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 2;
    pInfo.name     = "FAN_SPEED";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 3;
    pInfo.name     = "SOUND";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 4;
    pInfo.name     = "LIGHT";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 5;
    pInfo.name     = "WORK_MODE";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 6;
    pInfo.name     = "SAFE_LOCK";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 7;
    pInfo.name     = "CONSUM_WARN";
    pInfo.isText   = true;
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 8;
    pInfo.name     = "TIME";
    pInfo.isText   = false;
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 9;
    pInfo.name     = "PM25";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    pInfo.appid    = 10;
    pInfo.name     = "RESET";
    PlatformPropertyRegister(g_ctx, NULL, &pInfo);

    PlatformPropertySetNumValue(g_ctx, g_devID, 1, 0); //close
    PlatformPropertySetNumValue(g_ctx, g_devID, 2, 3); //auto
    PlatformPropertySetNumValue(g_ctx, g_devID, 3, 0); //mute
    PlatformPropertySetNumValue(g_ctx, g_devID, 4, 0); //shut light
    PlatformPropertySetNumValue(g_ctx, g_devID, 5, 1); //auto work
    PlatformPropertySetNumValue(g_ctx, g_devID, 6, 1); //lock
    PlatformPropertySetTextValue(g_ctx, g_devID, 7, "0,1|2|3|");
    PlatformPropertySetNumValue(g_ctx, g_devID, 8, 91);
    PlatformPropertySetNumValue(g_ctx, g_devID, 9, 102);//pm2.5
    PlatformPropertySetNumValue(g_ctx, g_devID, 10, 0);//reset
}

static const char *g_resourceKeyword[2] = {"CONSUM_NAME", "REMAIN_LIFE"};
ROM_FUNC static void resourceConfig(void)
{
    PlatformResourceRegister(g_ctx, g_devID, "CONSUM", "CONSUM_ID", 0);
    PlatformResourceKeywordRegister(g_ctx, g_devID, "CONSUM", g_resourceKeyword[0], true);
    PlatformResourceKeywordRegister(g_ctx, g_devID, "CONSUM", g_resourceKeyword[1], false);

    /*��һ��������и�ID�µ�������Դ*/
    PCommonInfo_t info[2];
    info[0].isText = true;
    info[0].name = g_resourceKeyword[0];
    info[0].value.text = "test1";

    info[1].isText = false;
    info[1].name = g_resourceKeyword[1];
    info[1].value.num = 85;
    PlatformResourceItemAdd(g_ctx, g_devID, "CONSUM", 1, info, 2);
}


ROM_FUNC void DemoInit(void)
{

    PlatformInitialize();
    g_ctx = PlatformCtxCreate();
    if(g_ctx)
    {
        PlatformEventRegister(g_ctx, platformEventHandle);
        PlatformSetDeviceInfo(g_ctx, g_devID, g_devPin, "1001", "001.000.000.001", "yumair");
        propertiesConfig();
        resourceConfig();

        PlatformStart(g_ctx);
    }
}

ROM_FUNC static void simulatePM25(void)
{
    static uint32_t lastTime = 0;

    if(PlatformTime() - lastTime > 20000)
    {
        ulog("");
        PlatformPropertySetNumValue(g_ctx, g_devID, 9, 100 + (PUtcTime() % 59));//pm2.5
        lastTime = PlatformTime();
    }
}

ROM_FUNC void DemoPoll(void)
{
    PlatformPoll(g_ctx);
    simulatePM25();
}

#endif