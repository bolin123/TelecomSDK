#include "PlatformMc.h"
#include "McCMD.h"
#include "PUtil.h"
#include "PropertyManager.h"
#include "ConnectionManager.h"
#include "Util/PList.h"
#include "Util/json.h"

typedef struct PMessageList_st
{
    puint16_t sequence;
    char *data;
    puint16_t len;
    puint8_t retry;
    ptime_t lastSendTime;
    PrivateCtx_t *ctx;
    PLIST_ENTRY(struct PMessageList_st);
}PMessageList_t;

static PMessageList_t g_messageList;

static void postResources(PrivateCtx_t *ctx, ResourceInfo_t *resources, const char *did, pbool_t force);
static void postProperties(PrivateCtx_t *ctx, PMProperty_t *properties, const char *did, pbool_t force);

_ptag static pbool_t isAckedCode(puint16_t code)
{
    if(code == MC_CMD_HEARTBEAT_ACK
        || code == MC_CMD_LOGIN_ACK
        || code == MC_CMD_CONNECT_ACK
        || code == MC_CMD_POST_ACK
        || code == MC_CMD_WARNING_ACK
        || code == MC_CMD_FAULT_ACK
        || code == MC_CMD_SUBAUTH_ACK
        || code == MC_CMD_SUBBIND_ACK
        || code == MC_CMD_SUBUNBIND_REPORT_ACK
        || code == MC_CMD_SUBONLINE_ACK
        || code == MC_CMD_DEV_VERSION_ACK
        || code == MC_CMD_UPGRADE_RESULT_ACK
        || code == MC_CMD_RESOURCE_REPORT_ACK
        || code == MC_CMD_EVENT_REPORT_ACK
        || code == MC_CMD_TTS_ACK
        || code == MC_CMD_VOICE_CTRL_ACK)
    {
        return ptrue;
    }
    return pfalse;
}

_ptag static pbool_t findAndDelMessage(puint16_t sequence)
{
    PMessageList_t *msg;

    plog("sequence = %d", sequence);
    PListForeach(&g_messageList, msg)
    {
        if(msg->sequence == sequence)
        {
            PListDel(msg);
            if(msg->data)
            {
                free(msg->data);
                msg->data = PNULL;
            }
            free(msg);
            return ptrue;
        }
    }
    plog("not found!");
    return pfalse;
}

_ptag static void messageSend(PrivateCtx_t *ctx, const char *data, puint32_t len, puint16_t sequence)
{
    PMessageList_t *message = (PMessageList_t *)malloc(sizeof(PMessageList_t));

    if(message)
    {
        message->data = (char *)malloc(len + 1);//'\0' for printf
        if(message->data)
        {
            memcpy(message->data, data, len);
            message->data[len] = '\0';
            message->len = len;
            message->retry = 0;
            message->sequence = sequence;
            message->lastSendTime = 0;
            message->ctx = ctx;
            PListAdd(&g_messageList, message);
        }
    }
}

_ptag static void sendLoginData(PrivateCtx_t *ctx, int cmd, const char *data)
{
    char *buff;
    pint32_t len;
    char *key = ctx->pin;
    char *iv = key + 16;
    json_item_t *message = json_item_create(JSON_OBJECT, PNULL);
    char *dataStr = PUtilAESEncodeBase64(data, strlen(data), key, iv);

    json_item_add_subitem(message, J_CREATE_I(MC_CODE, cmd));
    json_item_add_subitem(message, J_CREATE_S(MC_DEVID, ctx->did));
    json_item_add_subitem(message, J_CREATE_S(MC_DATA, dataStr));
    free(dataStr);

    len = json_item_eval_parse_text_len(message) + 6;//6 = '\0' + CTS + "\r\n"
    buff = (char *)malloc(len);
    if(buff)
    {
        memset(buff, 0, len);
        buff[0] = 'C';
        buff[1] = 'T';
        buff[2] = 'S';
        json_item_parse_to_text(message, &buff[3]);
        json_item_destroy(message);
        strcat(buff, "\r\n");

        //CMServerSendData(buff, len);
        messageSend(ctx, buff, strlen(buff), ctx->sendsn);

        free(buff);
    }
}

_ptag static void sendJsonData(PrivateCtx_t *ctx, int cmd, json_item_t *contents, pbool_t needReply)
{
    char *buff;
    pint32_t len;
    char *data, *dataStr;
    char *key = ctx->sessionkey;
    char *iv = key;

    //data from json to string
    pint32_t msglen = json_item_eval_parse_text_len(contents);
    data = (char *)malloc(msglen + 1);
    json_item_parse_to_text(contents, data);
    json_item_destroy(contents);
    //encode
    dataStr = PUtilAESEncodeBase64(data, strlen(data), key, iv);
    pprintf("%s\r\n", data);
    free(data);

    //full package
    json_item_t *message = json_item_create(JSON_OBJECT, PNULL);
    json_item_add_subitem(message, J_CREATE_I(MC_CODE, cmd));
    json_item_add_subitem(message, J_CREATE_S(MC_TOKEN, ctx->token));
    json_item_add_subitem(message, J_CREATE_S(MC_DATA, dataStr));
    free(dataStr);

    len = json_item_eval_parse_text_len(message) + 6;//6 = '\0' + CTS + "\r\n"
    buff = (char *)malloc(len); //CTS+\r\n
    if(buff)
    {
        memset(buff, 0, len);
        buff[0] = 'C';
        buff[1] = 'T';
        buff[2] = 'S';
        json_item_parse_to_text(message, &buff[3]);
        json_item_destroy(message);
        strcat(buff, "\r\n");

        if(needReply)
        {
            messageSend(ctx, buff, strlen(buff), ctx->sendsn);
        }
        else
        {
            CMServerSendData(ctx, buff, strlen(buff));
        }

        free(buff);
    }

}

_ptag static json_item_t *createMessageHead(const char *did, puint16_t sendsn, pbool_t needDid)
{
    char sqnStr[16] = "";
    json_item_t *post = json_item_create(JSON_OBJECT, PNULL);

    sprintf(sqnStr, "%d", sendsn);
    json_item_add_subitem(post, J_CREATE_S(MC_SEQUENCE, sqnStr));
    if(needDid)
    {
        json_item_add_subitem(post, J_CREATE_S(MC_DEVID, did));
    }
    json_item_add_subitem(post, J_CREATE_I(MC_TIME, PUtcTime()));
    return post;
}

_ptag void PlatformMcDisconnectHandle(PrivateCtx_t *ctx)
{
    PMessageList_t *msg;
    PMSubDevice_t *subDev;

    plog("");
    /*clear send list cache*/
    PListForeach(&g_messageList, msg)
    {
        PListDel(msg);
        if(msg->data)
        {
            free(msg->data);
            msg->data = PNULL;
        }
        free(msg);
        msg = PNULL;
    }

    /* TODO:reset sub device authentication flag*/
    PListForeach(&ctx->subDevices, subDev)
    {
        subDev->authStatus = SUBDEV_AUTH_NONE;
    }

}

_ptag static void messageSendPoll(void)
{
    PMessageList_t *msg = PListFirst(&g_messageList);

    if(msg)
    {
        if(msg->lastSendTime == 0 || PTimeHasPast(msg->lastSendTime, 10000))
        {
            if(msg->retry >= 3)
            {
                findAndDelMessage(msg->sequence);
                PlatformMcDisconnectHandle(msg->ctx);
                CMServerReconnect(msg->ctx);
            }
            else
            {
                CMServerSendData(msg->ctx, msg->data, msg->len);
                msg->lastSendTime = PlatformTime();
                msg->retry++;
            }
        }
    }
}

_ptag static void controlReply(PrivateCtx_t *ctx, puint16_t code, pint32_t result, const char *did, const char *subDid)
{
    json_item_t *reply = createMessageHead(did, ctx->recvsn, ptrue);

    json_item_add_subitem(reply, J_CREATE_I(MC_RESULT, result));
    if(subDid)
    {
        json_item_add_subitem(reply, J_CREATE_S(MC_SUBDEV_ID, subDid));
    }

    sendJsonData(ctx, code, reply, pfalse);
}

_ptag static void controlHandle(PrivateCtx_t *ctx, const char *did, json_item_t *msg)
{
    int i;
    char *setName;
    char *cmdName, *cmdParam;
    PMProperty_t *property;
    PPropertyCtrl_t pctrl;
    char *sid = J_SUB_VALUE_BY_NAME_S(msg, MC_SERIAL_ID);
    json_item_t *cmd = json_item_get_subitem_by_name(msg, "cmd");

    if(cmd)
    {
        pctrl.did = did;
        pctrl.sid = (puint16_t)strtol(sid, PNULL, 10);
        pctrl.propertyNum = json_item_get_subitem_count(cmd);
        pctrl.property = malloc(sizeof(PPropertyValue_t) * pctrl.propertyNum);
        if(pctrl.property)
        {
            memset(pctrl.property, 0, sizeof(PPropertyValue_t) * pctrl.propertyNum);
            for(i = 0; i < pctrl.propertyNum; i++)
            {
                json_item_t *item = json_item_get_subitem_by_index(cmd, i);
                setName = J_SUB_VALUE_BY_NAME_S(item, MC_CMDNAME);
                cmdParam = J_SUB_VALUE_BY_NAME_S(item, MC_CMDPARAM);
                if(setName)
                {
                    cmdName = strstr(setName, "SET_");
                    if(cmdName)
                    {
                        cmdName = cmdName + strlen("SET_");
                    }
                }
                else
                {
                    cmdName = PNULL;
                }
                property = PMFindPropertyByName(ctx, did, cmdName);
                if(property)
                {
                    pctrl.property[i].appid = property->appID;
                    if(property->type == PROPERTY_TYPE_NUM)
                    {
                        pctrl.property[i].isText = pfalse;
                        pctrl.property[i].value.num = (int)strtol(cmdParam, PNULL, 10);
                    }
                    else
                    {
                        //pctrl.property[i].value.text = (char *)malloc(strlen(cmdParam) + 1);
                        //pctrl.property[i].value.text[0] = '\0';
                        //strcpy(pctrl.property[i].value.text, cmdParam);
                        pctrl.property[i].value.text = cmdParam;
                        pctrl.property[i].isText = ptrue;
                    }
                }
                else
                {
                    pctrl.property[i].appid = PROPERTY_INVALID_APPID;
                    pctrl.property[i].isText = pfalse;
                    pctrl.property[i].value.num = (int)strtol(cmdParam, PNULL, 10);
                    plog("not found dev[%s] have property named[%s]", did, cmdName);
                }
            }

            PPrivateEventEmit(ctx, PEVENT_PROPERTY_CONTRL, (void *)&pctrl);

            free(pctrl.property);
        }
    }
}

_ptag static void subDeviceUnbind(PrivateCtx_t *ctx, const char *subDid)
{
    if(subDid)
    {
        PPrivateSubDeviceDel(ctx, subDid);
        PPrivateEventEmit(ctx, PEVENT_SUBDEV_UNBINDED, (void *)subDid);
        controlReply(ctx, MC_CMD_SUBUNBIND - 1, 0, ctx->did, subDid);
    }
    else
    {
        controlReply(ctx, MC_CMD_SUBUNBIND - 1, 100001, ctx->did, subDid);
    }
}

_ptag static void upgradeNoticeHandle(PrivateCtx_t *ctx, const char *did, json_item_t *msg)
{
    int i, pos = 0;
    char *version;
    PUpgradeNotice_t notice;
    json_item_t *device = json_item_get_subitem_by_name(msg, "device");
    json_item_t *modules = json_item_get_subitem_by_name(msg, "modules");

    notice.did = did;
    notice.infoNum = 0;
    if(modules)
    {
        notice.infoNum = json_item_get_subitem_count(modules);
    }
    notice.info = (PUpgradeInfo_t *)malloc(sizeof(PUpgradeInfo_t) * (notice.infoNum + 1)); //默认设备有升级信息

    if(notice.info)
    {
        if(device)
        {
            version = J_SUB_VALUE_BY_NAME_S(device, MC_VERSION);
            if(version)
            {
                notice.infoNum++;
                notice.info[pos].type = 0;
                notice.info[pos].name = PNULL;
                notice.info[pos].version = version;
                notice.info[pos].url = J_SUB_VALUE_BY_NAME_S(device, "url");
                notice.info[pos].md5 = J_SUB_VALUE_BY_NAME_S(device, "md5");
                pos++;
            }
        }

        if(modules)
        {
            for(i = 0; i < notice.infoNum; i++)
            {
                json_item_t *item = json_item_get_subitem_by_index(modules, i);
                notice.info[pos + i].type = 1;
                notice.info[pos + i].name = J_SUB_VALUE_BY_NAME_S(item, "name");
                notice.info[pos + i].url = J_SUB_VALUE_BY_NAME_S(item, "url");
                notice.info[pos + i].version = J_SUB_VALUE_BY_NAME_S(item, MC_VERSION);
                notice.info[pos + i].md5 = J_SUB_VALUE_BY_NAME_S(item, "md5");
            }
        }

        PPrivateEventEmit(ctx, PEVENT_OTA_NOTICE, (void *)&notice);
        free(notice.info);
    }

    controlReply(ctx, PEVENT_OTA_NOTICE - 1, 0, ctx->did, PNULL);
}

_ptag static void resourceManagerHandle(PrivateCtx_t *ctx, const char *did, json_item_t *msg)
{
    int i, j, m, n;
    char *idvalue;
    int rscSerialsNum, rscInfoNum, rscSerialNum;
    char *rscName, *idName;
    json_item_t *item;
    json_item_t *resources, *resource, *resourceSerial, *rscInfo;
    ResourceInfo_t *rInfoItem;
    ResourceInfo_t *rInfo = PMFindResourceInfoByDid(ctx, did);

    json_item_t *resourceSerials = json_item_get_subitem_by_name(msg, MC_RESOURCE_SERIALS);//resourceSerials[]

    if(resourceSerials)
    {
        rscSerialsNum = json_item_get_subitem_count(resourceSerials);

        for(i = 0; i < rscSerialsNum; i++)
        {
            resources = json_item_get_subitem_by_index(resourceSerials, i);
            resourceSerial = json_item_get_subitem_by_name(resources, MC_RESOURCE_SERIAL);//resourceSerial[]
            if(resourceSerial)
            {
                rscSerialNum = json_item_get_subitem_count(resourceSerial);
                for(n = 0; n < rscSerialNum; n++)
                {
                    resource = json_item_get_subitem_by_index(resourceSerial, n);
                    rscName = J_SUB_VALUE_BY_NAME_S(resource, MC_RESOURCE_NAME); //resourceName
                    if(rscName)
                    {
                        rscInfo = json_item_get_subitem_by_name(resource, MC_RESOURCE_INFO); //resourceInfo[]
                        rscInfoNum = json_item_get_subitem_count(rscInfo);
                        for(j = 0; j < rscInfoNum; j++)
                        {
                            item = json_item_get_subitem_by_index(rscInfo, j);
                            char *modeVal = J_SUB_VALUE_BY_NAME_S(item, "MODE");

                            if(modeVal)
                            {
                                ResourceMode_t mode = (ResourceMode_t)atoi(modeVal);
                                PListForeach(rInfo, rInfoItem)
                                {
                                    puint16_t idNum;
                                    PResources_t rscArgs;
                                    idvalue = J_SUB_VALUE_BY_NAME_S(item, rInfoItem->idName);
                                    if(idvalue)
                                    {
                                        idNum = (puint16_t)strtol(idvalue, PNULL, 10);

                                        rscArgs.did = did;
                                        rscArgs.rscName = rscName;
                                        rscArgs.rscID = idNum;

                                        puint8_t count = 0;
                                        for(m = 0; m < rInfoItem->keyNum; m++)
                                        {
                                            if(J_SUB_VALUE_BY_NAME_S(item, rInfoItem->key[m]))
                                            {
                                                count++;
                                            }
                                        }

                                        rscArgs.infoNum = count;
                                        rscArgs.info = (PCommonInfo_t *)malloc(count * sizeof(PCommonInfo_t));
                                        count = 0;
                                        if(rscArgs.info)
                                        {
                                            for(m = 0; m < rInfoItem->keyNum; m++)
                                            {
                                                char *temp = J_SUB_VALUE_BY_NAME_S(item, rInfoItem->key[m]);
                                                if(temp)
                                                {
                                                    rscArgs.info[count].name = rInfoItem->key[m];
                                                    if(rInfoItem->keyValueIsText[m])
                                                    {
                                                        rscArgs.info[count].isText = ptrue;
                                                        rscArgs.info[count].value.text = temp;
                                                    }
                                                    else
                                                    {
                                                        rscArgs.info[count].isText = pfalse;
                                                        rscArgs.info[count].value.num = (int)strtol(temp, PNULL, 10);
                                                    }
                                                    count++;
                                                }
                                            }
                                        }

                                        if(mode == RESOURCE_MODE_ADD)
                                        {
                                            PPrivateEventEmit(ctx, PEVENT_RESOURCE_ADD, (void *)&rscArgs);
                                        }
                                        else if(mode == RESOURCE_MODE_DEL)
                                        {
                                            PPrivateEventEmit(ctx, PEVENT_RESOURCE_DEL, (void *)&rscArgs);
                                        }
                                        else
                                        {
                                            PPrivateEventEmit(ctx, PEVENT_RESOURCE_SET, (void *)&rscArgs);
                                        }
                                        if(rscArgs.info)
                                        {
                                            free(rscArgs.info);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
    }
}

_ptag static void parseCtrolMessage(PrivateCtx_t *ctx, char *did, int cmd, json_item_t *msg)
{
    //char *did;
    PMProperty_t *property;
    ResourceInfo_t *resource;

    plog("cmd = %d", cmd);
    //did = J_SUB_VALUE_BY_NAME_S(msg, MC_DEVID);
    switch(cmd)
    {
    case MC_CMD_QUERY:
        property = PMFindPropertyHeadByDid(ctx, did);
        resource = PMFindResourceInfoByDid(ctx, did);
        if(property)
        {
            controlReply(ctx, cmd - 1, 0, did, PNULL);
            postProperties(ctx, property, did, ptrue);
            postResources(ctx, resource, did, ptrue);
        }
        else
        {
            controlReply(ctx, cmd - 1, 200001, did, PNULL);
        }
        break;
    case MC_CMD_CONTROL:
        controlHandle(ctx, did, msg);
        controlReply(ctx, cmd - 1, 0, did, PNULL);
        break;
    case MC_CMD_SUBUNBIND:
        subDeviceUnbind(ctx, J_SUB_VALUE_BY_NAME_S(msg, MC_SUBDEV_ID));
        break;
    case MC_CMD_UPGRADE_NOTICE:
        upgradeNoticeHandle(ctx, did, msg);
        break;
    case MC_CMD_RESOURCE_MANAGER:
        resourceManagerHandle(ctx, did, msg);
        break;
    default:
        break;
    }
}


_ptag static void subDeviceStatusReport(PrivateCtx_t *ctx, const char *subid, pbool_t online)
{
    json_item_t *status = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);

    json_item_add_subitem(status, J_CREATE_S(MC_SUBDEV_ID, subid));
    json_item_add_subitem(status, J_CREATE_I(MC_SUBDEV_ONLINE, online));

    sendJsonData(ctx, MC_CMD_SUBONLINE, status, ptrue);
}

_ptag static void postVersion(PrivateCtx_t *ctx, const char *did, const char *version, const char *model, ModulesVersion_t *modules)
{
    json_item_t *ver = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_add_subitem(ver, J_CREATE_S(MC_MODEL, model));

    json_item_t *dev = json_item_create(JSON_OBJECT, "device");
    json_item_add_subitem(dev, J_CREATE_S(MC_VERSION, version));

    json_item_t *mod = json_item_create(JSON_ARRAY, "modules");
    ModulesVersion_t *module;
    PListForeach(modules, module)
    {
        json_item_t *node = json_item_create(JSON_OBJECT, PNULL);

        json_item_add_subitem(node, J_CREATE_S("name", module->name));
        json_item_add_subitem(node, J_CREATE_S(MC_VERSION, module->version));
        json_item_add_subitem(mod, node);
    }
    json_item_add_subitem(ver, mod);
    json_item_add_subitem(ver, dev);
    plog("");

    sendJsonData(ctx, MC_CMD_DEV_VERSION, ver, ptrue);
}

_ptag static void sudDeviceAuthResult(PrivateCtx_t *ctx, const char *did, pbool_t success)
{
    PMSubDevice_t *subDev;

    if(did)
    {
        subDev = PPrivateGetSubDevice(ctx, did);
        if(subDev)
        {
            if(success)
            {
                subDev->authStatus = SUBDEV_AUTH_SUCCESS;
                subDeviceStatusReport(ctx, subDev->did, subDev->online);
                /*
                if(subDev->online)
                {
                    postProperties(ctx, &subDev->property, subDev->did, ptrue);
                    postResources(ctx, &subDev->resource, subDev->did, ptrue);
                }
                */
                postVersion(ctx, subDev->did, subDev->version, subDev->model, &subDev->modules);
            }
            else
            {
                subDev->authStatus = SUBDEV_AUTH_FAILED;
                PPrivateEventEmit(ctx, PEVENT_SUBDEV_AUTH_FAILED, (void *)subDev->did);
            }
        }
    }
}

_ptag void PlatformMcOTAResultReport(PrivateCtx_t *ctx, pbool_t success)
{
    int status = success ? 0 : 1; //成功 = 0， 失败 = 1
    json_item_t *result = createMessageHead(ctx->ota.did, ++ctx->sendsn, ptrue);

    json_item_add_subitem(result, J_CREATE_I("type", ctx->ota.type));
    if(ctx->ota.type == 1)
    {
        json_item_add_subitem(result, J_CREATE_S("moduleName", ctx->ota.name));
    }
    json_item_add_subitem(result, J_CREATE_I("status", status));
    json_item_add_subitem(result, J_CREATE_S("version", ctx->ota.version));

    sendJsonData(ctx, MC_CMD_UPGRADE_RESULT, result, ptrue);
}

_ptag void PlatformMcErrReport(PrivateCtx_t *ctx, const char *did, PErrorReport_t *err)
{
    json_item_t *errReport = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *error = json_item_create(JSON_OBJECT, "error");

    plog("");
    if(err)
    {
        json_item_add_subitem(error, J_CREATE_I(MC_SERIAL_ID, err->serialId));
        json_item_add_subitem(error, J_CREATE_I(MC_ERROR_CODE, err->errCode));
        json_item_add_subitem(error, J_CREATE_S(MC_ERROR_INFO, err->errInfo));
        json_item_add_subitem(error, J_CREATE_I(MC_ERROR_TIME, err->time));
    }
    json_item_add_subitem(errReport, error);

    sendJsonData(ctx, MC_CMD_FAULT, errReport, ptrue);
}

_ptag void PlatformMcStatusAlarm(PrivateCtx_t *ctx, const char *did, PStatusAlarm_t *alarm)
{
    json_item_t *statusAlarm = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *alarmInfo = json_item_create(JSON_OBJECT, "alarm");

    plog("");
    if(alarm)
    {
        json_item_add_subitem(alarmInfo, J_CREATE_I(MC_SERIAL_ID, alarm->serialId));
        json_item_add_subitem(alarmInfo, J_CREATE_S(MC_STATUS_NAME, alarm->statusInfo.name));
        if(alarm->statusInfo.isText)
        {
            json_item_add_subitem(alarmInfo, J_CREATE_S(MC_CUR_STATUS_VALUE, alarm->statusInfo.value.text));
        }
        else
        {
            json_item_add_subitem(alarmInfo, J_CREATE_I(MC_CUR_STATUS_VALUE, alarm->statusInfo.value.num));
        }
        json_item_add_subitem(alarmInfo, J_CREATE_S("dscp", alarm->description));
        json_item_add_subitem(alarmInfo, J_CREATE_I("alarmTime", alarm->time));
    }
    json_item_add_subitem(statusAlarm, alarmInfo);

    sendJsonData(ctx, MC_CMD_WARNING, statusAlarm, ptrue);
}

//设备信息上报
_ptag void PlatformMcEventInfoReport(PrivateCtx_t *ctx, const char *did, PEventInfo_t *event)
{
    puint8_t i;
    json_item_t *msg = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *jevent = json_item_create(JSON_ARRAY, "event");
    plog("");

    json_item_t *eventCtx = json_item_create(JSON_OBJECT, PNULL);
    json_item_add_subitem(eventCtx, J_CREATE_I(MC_EVENT_ID, 1));
    json_item_add_subitem(eventCtx, J_CREATE_I(MC_SERIAL_ID, event->serialID));
    json_item_add_subitem(eventCtx, J_CREATE_S(MC_EVENT_NAME, event->name));

    json_item_t *eventInfo = json_item_create(JSON_ARRAY, MC_EVENT_INFO);
    json_item_t *eventItem = json_item_create(JSON_OBJECT, PNULL);
    for(i = 0; i < event->infoNum; i++)
    {
        if(event->info[i].isText)
        {
            json_item_add_subitem(eventItem, J_CREATE_S(event->info[i].name, event->info[i].value.text));
        }
        else
        {

            json_item_add_subitem(eventItem, J_CREATE_I(event->info[i].name, event->info[i].value.num));
        }
    }
    json_item_add_subitem(eventInfo, eventItem);
    json_item_add_subitem(eventCtx, eventInfo);

    json_item_add_subitem(jevent, eventCtx);
    json_item_add_subitem(msg, jevent);

    sendJsonData(ctx, MC_CMD_EVENT_REPORT, msg, ptrue);
}

_ptag void PlatformMcTTS(PrivateCtx_t *ctx, const char *did, PTTSParameter_t *param)
{
    json_item_t *tts = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *ttsParam = json_item_create(JSON_OBJECT, MC_TTS_PARAM);

    plog("");
    if(param)
    {
        json_item_add_subitem(ttsParam, J_CREATE_S("content", param->content));
        if(param->anchor)
        {
            json_item_add_subitem(ttsParam, J_CREATE_S("anchor", param->anchor));
        }
        json_item_add_subitem(ttsParam, J_CREATE_I("speed", param->speed));
        json_item_add_subitem(ttsParam, J_CREATE_I("volume", param->volume));
        json_item_add_subitem(ttsParam, J_CREATE_I("pitch", param->pitch));
        json_item_add_subitem(ttsParam, J_CREATE_S(MC_VOICE_CTRL_FORMAT, param->format));
        json_item_add_subitem(ttsParam, J_CREATE_I(MC_VOICE_CTRL_RATE, param->rate));
        json_item_add_subitem(ttsParam, J_CREATE_I(MC_VOICE_CTRL_CHANNEL, param->channel));
    }
    json_item_add_subitem(tts, ttsParam);
    sendJsonData(ctx, MC_CMD_TTS, tts, ptrue);
}

_ptag void PlatformMcVoiceControl(PrivateCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum)
{
    int i;
    json_item_t *voiceCtrol = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *ctrlParam = json_item_create(JSON_OBJECT, MC_VOICE_CTRL_PARAM);

    plog("");
    if(param)
    {
        json_item_add_subitem(ctrlParam, J_CREATE_S(MC_VOICE_CTRL_FORMAT, param->format));
        json_item_add_subitem(ctrlParam, J_CREATE_I(MC_VOICE_CTRL_RATE, param->rate));
        json_item_add_subitem(ctrlParam, J_CREATE_I(MC_VOICE_CTRL_CHANNEL, param->channel));
        json_item_add_subitem(ctrlParam, J_CREATE_S(MC_VOICE_CTRL_VOICEID, param->voiceID));
        json_item_add_subitem(ctrlParam, J_CREATE_I("sn", param->sn));
        json_item_add_subitem(ctrlParam, J_CREATE_I("isLast", isLast));
        json_item_add_subitem(ctrlParam, J_CREATE_S(MC_VOICE_CTRL_AUDIODATA, data));
        if(ext)
        {
            json_item_t *extensions = json_item_create(JSON_ARRAY, MC_VOICE_CTRL_EXTENSIONS);
            for(i = 0; i < extNum; i++)
            {
                json_item_t *keyvalue = json_item_create(JSON_OBJECT, PNULL);
                json_item_add_subitem(keyvalue, J_CREATE_S("key", ext[i].key));
                json_item_add_subitem(keyvalue, J_CREATE_S("value", ext[i].value));
                json_item_add_subitem(extensions, keyvalue);
            }
            json_item_add_subitem(ctrlParam, extensions);
        }
    }

    json_item_add_subitem(voiceCtrol, ctrlParam);

    sendJsonData(ctx, MC_CMD_VOICE_CTRL, voiceCtrol, ptrue);
}

_ptag void PlatformMcSubDeviceUnbindReport(PrivateCtx_t *ctx, const char *did)
{
    json_item_t *unbind = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);
    plog("did:%s", did);
    json_item_add_subitem(unbind, J_CREATE_S(MC_SUBDEV_ID, did));

    sendJsonData(ctx, MC_CMD_SUBUNBIND_REPORT, unbind, ptrue);
}

_ptag int PlatformMcSubDeviceOnOffline(PrivateCtx_t *ctx, const char *did, pbool_t online)
{
    PMSubDevice_t *subDev = PPrivateGetSubDevice(ctx, did);

    if(subDev)
    {
        subDev->online = online;
        subDev->onlineAcked = pfalse;
        if(CMServerConnected(ctx))
        {
            subDeviceStatusReport(ctx, subDev->did, online);
        }
        return 0;
    }
    return -1;
}

_ptag static void parseReplyMessage(PrivateCtx_t *ctx, char *did, int cmd, json_item_t *msg)
{
    char *tcphost;
    char ip[16] = "";
    puint16_t port;
    char *tmp, *subDeviceId;
    char *authResult;
    puint32_t result;
    PTTSResult_t tts;
    PMSubDevice_t *subDev;
    PVoiceCtrlResult_t voiceCtrlResult;
    json_item_t *item;

    tmp = J_SUB_VALUE_BY_NAME_S(msg, MC_RESULT);
    result = (puint32_t)strtol(tmp, PNULL, 10);

    if(result != 0)
    {
        //event error
        plog("error = %d", result);
        return;
    }

    plog("cmd = %d", cmd);
    switch(cmd)
    {
    case MC_CMD_HEARTBEAT_ACK:
        break;
    case MC_CMD_LOGIN_ACK:
        strncpy(ctx->sessionkey, J_SUB_VALUE_BY_NAME_S(msg, MC_SESSIONKEY), PLATFORM_SESSIONKEY_LEN);
        strncpy(ctx->token, J_SUB_VALUE_BY_NAME_S(msg, MC_TOKEN), PLATFORM_TOKEN_LEN);
        tcphost = J_SUB_VALUE_BY_NAME_S(msg, MC_TCPHOST);
        tmp = strchr(tcphost, ':');
        memcpy(ip, tcphost, tmp - tcphost);
        port = (puint16_t)strtol(tmp + 1, PNULL, 10);
        CMServerLoginSuccess(ctx, ip, port);
        break;
    case MC_CMD_CONNECT_ACK:
        tmp = J_SUB_VALUE_BY_NAME_S(msg, MC_HEARTBEAT);
        if(tmp)
        {
            ctx->hbIntervel = (puint16_t)strtol(tmp, PNULL, 10);
        }
        ctx->lastHbSendTime = PlatformTime();
        CMServerOnline(ctx);
        break;
    case MC_CMD_POST_ACK:
        //send resource
        //check sub device
        break;
    case MC_CMD_WARNING_ACK:
        break;
    case MC_CMD_FAULT_ACK:
        break;
    case MC_CMD_SUBAUTH_ACK:
        authResult = J_SUB_VALUE_BY_NAME_S(msg, MC_AUTH_RESULT);
        sudDeviceAuthResult(ctx, J_SUB_VALUE_BY_NAME_S(msg, MC_SUBDEV_ID), (authResult[0] == '0'));
        break;
    case MC_CMD_SUBBIND_ACK:
        break;
    case MC_CMD_SUBUNBIND_REPORT_ACK:
        break;
    case MC_CMD_SUBONLINE_ACK:
        subDeviceId = J_SUB_VALUE_BY_NAME_S(msg, MC_SUBDEV_ID);
        if(subDeviceId)
        {
            subDev = PPrivateGetSubDevice(ctx, subDeviceId);
            if(subDev && subDev->online)
            {
                subDev->onlineAcked = ptrue;
                subDev->onlineTime = PlatformTime();
                subDev->needPost = ptrue;
            }
        }
        break;
    case MC_CMD_DEV_VERSION_ACK:
        break;
    case MC_CMD_UPGRADE_RESULT_ACK:
        break;
    case MC_CMD_EVENT_REPORT_ACK:
        break;
    case MC_CMD_RESOURCE_REPORT_ACK:
        break;
    case MC_CMD_TTS_ACK:
        tts.did = did;
        item = json_item_get_subitem_by_name(msg, MC_TTS_RESULT);
        tts.content = J_SUB_VALUE_BY_NAME_S(item, MC_VOICE_CTRL_AUDIODATA);
        PPrivateEventEmit(ctx, PEVENT_TTS_RESULT, (void *)&tts);
        break;
    case MC_CMD_VOICE_CTRL_ACK:
        voiceCtrlResult.did = did;
        item = json_item_get_subitem_by_name(msg, MC_VOICE_CTRL_RESULT);
        voiceCtrlResult.rawText = J_SUB_VALUE_BY_NAME_S(item, "rawText");
        voiceCtrlResult.description = J_SUB_VALUE_BY_NAME_S(item, "resultDesc");
        PPrivateEventEmit(ctx, PEVENT_VOICE_CONTROL_RESULT, (void *)&voiceCtrlResult);
        break;
    default:
        break;
    }
}

_ptag void PlatformTestCmdRecv(PrivateCtx_t *ctx, const char *msg, puint32_t len)
{
    puint16_t cmd;
    json_item_t *contents = json_item_parse_from_text(msg);
    char *did = J_SUB_VALUE_BY_NAME_S(contents, MC_DEVID);
    char *code = J_SUB_VALUE_BY_NAME_S(contents, MC_CODE);
    if(code)
    {
        cmd = (puint16_t)strtol(code, PNULL, 10);
    }

    parseCtrolMessage(ctx, did, cmd, contents);
}

_ptag void PlatformMcRecv(PrivateCtx_t *ctx, const char *msg, puint32_t len)
{
    puint8_t i, count = 0;
    char *buff;
    char *newMsg = msg;
    char *data, *utcstr;
    puint8_t *contentsStr;
    int contentsLen;
    puint16_t codeNum;
    puint8_t *key, *iv;
    char *head, *end;
    json_item_t *pack, *contents, *code;

    pprintf("Recv:\r\n%s\r\n", msg);

    while(1)
    {
        newMsg = strstr(newMsg, MC_PROTO_END);
        if(newMsg)
        {
            newMsg += 2;
            count++;
        }
        else
        {
            break;
        }
    }

    head = msg;
    for(i = 0; i < count; i++)
    {
        head = strstr(head, MC_PROTO_HEAD);
        end = strstr(head, MC_PROTO_END);

        if(head && end)
        {
            buff = (char *)malloc(end - head + 1);
            memset(buff, 0, end - head + 1);
            memcpy(buff, head, end - head);
            head = end + 2;

            pack = json_item_parse_from_text(buff + 3);
            free(buff);

            code = json_item_get_subitem_by_name(pack, MC_CODE);
            codeNum = (puint16_t)strtol(json_item_get_string_value(code), PNULL, 10);
            data = J_SUB_VALUE_BY_NAME_S(pack, MC_DATA);

            if(codeNum == MC_CMD_LOGIN_ACK)
            {
                key = ctx->pin;
                iv = key + 16;
            }
            else
            {
                key = ctx->sessionkey;
                iv = key;
            }

            /*decode data segment*/
            contentsStr = PUtilBase64AESDecode(data, key, iv, &contentsLen);
            contentsStr[contentsLen] = '\0';
            pprintf("%s\r\n", contentsStr);
            json_item_destroy(pack);
            contents = json_item_parse_from_text(contentsStr);
            free(contentsStr);

            ctx->lastServerMsgTime = PlatformTime();

            /*get utc time*/
            utcstr = J_SUB_VALUE_BY_NAME_S(contents, MC_TIME);
            if(utcstr)
            {
                PPrivateEventEmit(ctx, PEVENT_TIMING_INFO, (void *)(puint32_t)strtol(utcstr, PNULL, 10));
            }

            json_item_t *seq = json_item_get_subitem_by_name(contents, MC_SEQUENCE);
            puint16_t sequence = (puint16_t)strtol(json_item_get_string_value(seq), PNULL, 10);

            char *did = J_SUB_VALUE_BY_NAME_S(contents, MC_DEVID);

            if(isAckedCode(codeNum))
            {
                findAndDelMessage(sequence);
                parseReplyMessage(ctx, did, codeNum, contents);
            }
            else
            {
                ctx->recvsn = sequence;
                parseCtrolMessage(ctx, did, codeNum, contents);
            }

            json_item_destroy(contents);

        }
    }


}

_ptag static void postProperties(PrivateCtx_t *ctx, PMProperty_t *properties, const char *did, pbool_t force)
{
    PMProperty_t *property;
    PMProperty_t *head = properties;
    json_item_t *post = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *statusSerials = json_item_create(JSON_ARRAY, MC_STATUS_SERIALS);

    plog("");
    PListForeach(head, property)
    {
        if(force || property->changed)
        {
            json_item_t *status = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(status, J_CREATE_I(MC_SERIAL_ID, property->serialID));

            json_item_t *statusSerial = json_item_create(JSON_ARRAY, MC_STATUS_SERIAL);

            json_item_t *node = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(node, J_CREATE_S(MC_STATUS_NAME, property->name));
            if(property->type == PROPERTY_TYPE_TEXT)
            {
                json_item_add_subitem(node, J_CREATE_S(MC_CUR_STATUS_VALUE, property->value.text));
            }
            else
            {
                json_item_add_subitem(node, J_CREATE_I(MC_CUR_STATUS_VALUE, property->value.num));
            }
            json_item_add_subitem(statusSerial, node);
            json_item_add_subitem(status, statusSerial);
            json_item_add_subitem(statusSerials, status);
            property->changed = pfalse;
        }
    }
    json_item_add_subitem(post, statusSerials);

    sendJsonData(ctx, MC_CMD_POST, post, ptrue);
}

_ptag static void postResources(PrivateCtx_t *ctx, ResourceInfo_t *resources, const char *did, pbool_t force)
{
    puint16_t i;
    pbool_t changed = pfalse;
    ResourceInfo_t *info;
    ResourceNode_t *nodeInfo;
    ResourceItems_t *item;
    ResourceInfo_t *rscHead = resources;

    if(PListFirst(rscHead) == PNULL)
    {
        return;
    }
    plog("");

    json_item_t *post = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *rscSerials = json_item_create(JSON_ARRAY, MC_RESOURCE_SERIALS);

    PListForeach(rscHead, info)
    {
        changed = pfalse;
        PListForeach(&info->items, item)
        {
            if(item->changed)
            {
                changed = ptrue;
                break;
            }
        }

        if(changed || force)
        {
            json_item_t *status = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(status, J_CREATE_I(MC_SERIAL_ID, info->serialID));

            json_item_t *rscSerial = json_item_create(JSON_ARRAY, MC_RESOURCE_SERIAL);
            json_item_t *name = json_item_create(JSON_OBJECT, PNULL);
            json_item_add_subitem(name, J_CREATE_S(MC_RESOURCE_NAME, info->rscName));

            json_item_t *rscInfo = json_item_create(JSON_ARRAY, MC_RESOURCE_INFO);

            PListForeach(&info->items, item)
            {
                if(item->changed || force)
                {
                    json_item_t *node = json_item_create(JSON_OBJECT, PNULL);
                    json_item_add_subitem(node, J_CREATE_I(info->idName, item->idValue));
                    PListForeach(&item->node, nodeInfo)
                    {
                        if(nodeInfo->type == PROPERTY_TYPE_TEXT)
                        {
                            json_item_add_subitem(node, J_CREATE_S(nodeInfo->name, nodeInfo->value.text));
                        }
                        else
                        {
                            json_item_add_subitem(node, J_CREATE_I(nodeInfo->name, nodeInfo->value.num));
                        }
                    }

                    json_item_add_subitem(node, J_CREATE_I("MODE", item->mode));

                    item->changed = pfalse;
                    json_item_add_subitem(rscInfo, node);
                }
            }
            json_item_add_subitem(name, rscInfo);
            json_item_add_subitem(rscSerial, name);
            json_item_add_subitem(status, rscSerial);
            json_item_add_subitem(rscSerials, status);
        }
    }

    json_item_add_subitem(post, rscSerials);

    sendJsonData(ctx, MC_CMD_RESOURCE_REPORT, post, ptrue);
}

_ptag void PlatformMcPostAll(PrivateCtx_t *ctx)
{
    plog("");
    postProperties(ctx, &ctx->properties, ctx->did, ptrue);
    postResources(ctx, &ctx->resources, ctx->did, ptrue);
    postVersion(ctx, ctx->did, ctx->version, ctx->model, &ctx->modules);
}

_ptag void checkPropertiesAndResources(PrivateCtx_t *ctx)
{
    puint16_t i;
    pbool_t changed = pfalse;
    PMProperty_t *property;
    ResourceInfo_t *resource;
    ResourceItems_t *resItem;

    if(CMServerConnected(ctx))//??? clients
    {
        PListForeach(&ctx->properties, property)
        {
            if(property->changed)
            {
                postProperties(ctx, &ctx->properties, ctx->did, pfalse);
                break;
            }
        }

        PListForeach(&ctx->resources, resource)
        {
            PListForeach(&resource->items, resItem)
            {
                if(resItem->changed)
                {
                    changed = ptrue;
                    break;
                }
            }

            if(changed)
            {
                postResources(ctx, &ctx->resources, ctx->did, pfalse);
                break;
            }
        }
    }
}

_ptag static void doAuthentication(PrivateCtx_t *ctx, PMSubDevice_t *subDev)
{
    json_item_t *auth = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);

    json_item_add_subitem(auth, J_CREATE_S(MC_SUBDEV_ID, subDev->did));
    json_item_add_subitem(auth, J_CREATE_S("pin", subDev->pin));

    sendJsonData(ctx, MC_CMD_SUBAUTH, auth, ptrue);
}

_ptag static void subDeviceAuthentication(PrivateCtx_t *ctx)
{
    PMSubDevice_t *subDev;

    if(CMServerConnected(ctx))
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            //if(subDev->online)
            {
                if(subDev->authStatus == SUBDEV_AUTH_START)
                {
                    break;
                }
                else if(subDev->authStatus == SUBDEV_AUTH_NONE)
                {
                    doAuthentication(ctx, subDev);
                    subDev->authStatus = SUBDEV_AUTH_START;
                    break;
                }
            }
        }
    }
}

_ptag static void subDeviceForcePost(PrivateCtx_t *ctx)
{
    PMSubDevice_t *subDev;

    if(CMServerConnected(ctx))
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            if(subDev->needPost && PTimeHasPast(subDev->onlineTime, 1000))
            {
                postProperties(ctx, &subDev->property, subDev->did, ptrue);
                postResources(ctx, &subDev->resource, subDev->did, ptrue);
                subDev->needPost = pfalse;
                break;
            }
        }
    }
}

_ptag static void subDevicePropertiesAndResources(PrivateCtx_t *ctx)
{
    puint16_t i;
    pbool_t changed = pfalse;
    PMSubDevice_t *subDev;
    PMProperty_t *property;
    ResourceInfo_t *resource;
    ResourceItems_t *resItem;

    if(CMServerConnected(ctx))
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            changed = pfalse;
            if(subDev->authStatus == SUBDEV_AUTH_SUCCESS && subDev->onlineAcked && !subDev->needPost)
            {
                PListForeach(&subDev->property, property)
                {
                    if(property->changed)
                    {
                        postProperties(ctx, &subDev->property, subDev->did, pfalse);
                        break;
                    }
                }

                PListForeach(&subDev->resource, resource)
                {

                    PListForeach(&resource->items, resItem)
                    {
                        if(resItem->changed)
                        {
                            changed = ptrue;
                            break;
                        }
                    }

                    if(changed)
                    {
                        postResources(ctx, &subDev->resource, subDev->did, pfalse);
                        break;
                    }
                }
            }
        }
    }
}

_ptag static void heartbeat(PrivateCtx_t *ctx)
{
    json_item_t *hb = createMessageHead(ctx->did, ++ctx->sendsn, pfalse);
    plog("");

    json_item_add_subitem(hb, J_CREATE_S(MC_TOKEN, ctx->token));
    sendJsonData(ctx, MC_CMD_HEARTBEAT, hb, ptrue);
}

_ptag static void heartBeatPoll(PrivateCtx_t *ctx)
{
    if(CMServerConnected(ctx))
    {
        if(PTimeHasPast(ctx->lastHbSendTime, ctx->hbIntervel * 1000))
        {
            heartbeat(ctx);
            ctx->lastHbSendTime = PlatformTime();
        }
    }
}

_ptag void PlatformMcLogin(PrivateCtx_t *ctx)
{
    char buff[256] = "";
    json_item_t *data = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);

    json_item_add_subitem(data, J_CREATE_S(MC_VERSION, PLATFORM_PROTOCOL_VERSION));
    json_item_parse_to_text(data, buff);
    json_item_destroy(data);
    plog("%s", buff);

    sendLoginData(ctx, MC_CMD_LOGIN, buff);
}

_ptag void PlatformMcOnline(PrivateCtx_t *ctx)
{
    char sqnStr[16] = "";
    json_item_t *data = json_item_create(JSON_OBJECT, PNULL);

    sprintf(sqnStr, "%d", ctx->sendsn);
    json_item_add_subitem(data, J_CREATE_S(MC_SEQUENCE, sqnStr));
    json_item_add_subitem(data, J_CREATE_S(MC_TOKEN, ctx->token));
    json_item_add_subitem(data, J_CREATE_S(MC_DEVVERSION, ctx->version));
    json_item_add_subitem(data, J_CREATE_S(MC_MODEL, ctx->model));
    json_item_add_subitem(data, J_CREATE_I(MC_TIME, PUtcTime()));
    plog("");

    sendJsonData(ctx, MC_CMD_CONNECT, data, ptrue);
}

_ptag static void jsonNameRegist(void)
{
    json_item_regist_value_name(MC_CODE);
    json_item_regist_value_name(MC_DATA);
    json_item_regist_value_name(MC_SEQUENCE);
    json_item_regist_value_name(MC_DEVID);
    json_item_regist_value_name(MC_VERSION);
    json_item_regist_value_name(MC_TIME);
    json_item_regist_value_name(MC_TOKEN);
    json_item_regist_value_name(MC_DEVVERSION);
    json_item_regist_value_name(MC_MODEL);
    json_item_regist_value_name(MC_TOKEN);

}

_ptag void PlatformMcInitialize(void)
{
    PListInit(&g_messageList);
    jsonNameRegist();
}

_ptag void PlatformMcPoll(PrivateCtx_t *ctx)
{
    messageSendPoll();
    heartBeatPoll(ctx);
    checkPropertiesAndResources(ctx);
    subDeviceAuthentication(ctx);
    subDevicePropertiesAndResources(ctx);
    subDeviceForcePost(ctx);
}

