#include "demo2.h"
#if defined(COMPILE_DEMO_2)
#include "Platform.h"
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
static bool g_online = false;

ROM_FUNC static void propertyContrl(PPropertyCtrl_t *pctrl)
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

ROM_FUNC static void otaNoticeHandle(PUpgradeNotice_t *notice)
{
    int i;
    PUpgradeInfo_t *info = NULL;

    if(notice)
    {
        ulog("upgrade device id:%s", notice->did);

        // 每次只能选择设备或一个模块固件进行升级
        for(i = 0; i < notice->infoNum; i++)
        {
            if(notice->info[i].name == NULL) //设备升级信息
            {
                ulog("device OTA version %s", notice->info[i].version);
                info = &notice->info[i];
            }
            else  //模块升级信息
            {
                ulog("module %s OTA version %s", notice->info[i].name, notice->info[i].version);
                info = &notice->info[i];
                break; //测试优先升级模块(根据实际升级策略选择)
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

    ulog("event = %d", event);
    switch(event)
    {
    case PEVENT_SERVER_ON_OFFLINE:     //服务器上、下线状态
        ulog("PEVENT_SERVER_ON_OFFLINE %d", (int)args);
        break;
    case PEVENT_PROPERTY_CONTRL:       //设备、子设备状态控制
        propertyContrl((PPropertyCtrl_t *)args);
        break;
    case PEVENT_RESOURCE_ADD:          //设备、子设备资源新增
        //todo: add resource
        PlatformResourceItemAdd(g_ctx, resource->did, resource->rscName, resource->rscID, resource->info, resource->infoNum);
        break;
    case PEVENT_RESOURCE_SET:          //设备、子设备资源设置
        //todo:set resource
        PlatformResourceItemSet(g_ctx, resource->did, resource->rscName, resource->rscID, resource->info, resource->infoNum);
        break;
    case PEVENT_RESOURCE_DEL:          //设备、子设备资源删除
        //todo:del resource
        PlatformResourceItemDel(g_ctx, resource->did, resource->rscName, resource->rscID);
        break;
    case PEVENT_SUBDEV_AUTH_FAILED:    //子设备鉴权失败
        ulog("subDevice %s PEVENT_SUBDEV_AUTH_FAILED!", (char *)args);
        break;
    case PEVENT_SUBDEV_UNBINDED:       //子设备被解绑
        ulog("subDevice %s PEVENT_SUBDEV_UNBINDED!", (char *)args);
        break;
    case PEVENT_TIMING_INFO:           //授时信息
        break;
    case PEVENT_OTA_NOTICE:            //升级通知
        otaNoticeHandle((PUpgradeNotice_t *)args);
        break;
    case PEVENT_OTA_DATA:              //升级数据
        otaDataHandle((POTADataArgs_t *)args);
        break;
    case PEVENT_OTA_FINISH:            //升级状态
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
    case PEVENT_VOICE_CONTROL_RESULT:  //语音控制结果
        break;
    case PEVENT_TTS_RESULT:
        break;
    default:
        break;
    }
}


/*子设备注册(门磁)

*/

ROM_FUNC static void subDeviceRegist(void)
{
    char *model = "doorSensor";
    char *version = "111.222.333.444";
    PlatformSubDeviceRegister(g_ctx, g_subDevID, g_subDevPin, model, version, "yumair");

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

    PlatformPropertySetNumValue(g_ctx, g_subDevID, 1, g_alarmEnable); //开启告警
    PlatformPropertySetNumValue(g_ctx, g_subDevID, 2, 0); //关门

    PlatformSubDeviceHeartbeat(g_ctx, g_subDevID);
    PlatformSubDeviceRSSIValue(g_ctx, g_subDevID, 35);
    PlatformSubDeviceBatteryRemain(g_ctx, g_subDevID, 95);
//    PlatformSubDeviceOnOffline(g_ctx, g_subDevID, true);//在线
}

ROM_FUNC static void propertiesConfig(void)
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

    PlatformPropertySetNumValue(g_ctx, g_devID, 1, 1); //烈焰红
    PlatformPropertySetNumValue(g_ctx, g_devID, 2, 0); //关闭灯光
    PlatformPropertySetNumValue(g_ctx, g_devID, 3, 0); //不播放声音
    PlatformPropertySetNumValue(g_ctx, g_devID, 4, 55); //音量大小
}

static uint32_t g_lastTime;
ROM_FUNC static void testSubDeviceReport(void)
{
    static bool doorOpened = false;
    if(g_online && g_alarmEnable && (PlatformTime() - g_lastTime) > 30000)
    {
        PlatformPropertySetNumValue(g_ctx, g_subDevID, 2, doorOpened); //关门
        doorOpened = !doorOpened;
        g_lastTime = PlatformTime();
    }
}

ROM_FUNC static void testOnoffLine(void)
{
    static uint32_t lastActTime = 0;
    if((PlatformTime() - lastActTime) > 65000)
    {
        g_online = !g_online;
        PlatformSubDeviceOnOffline(g_ctx, g_subDevID, g_online);//在线
        lastActTime = PlatformTime();
    }
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

        subDeviceRegist();

        PlatformStart(g_ctx);
    }

}

ROM_FUNC void DemoPoll(void)
{
    testOnoffLine();
    PlatformPoll(g_ctx);
    testSubDeviceReport();
}

#endif
