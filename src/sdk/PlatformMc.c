#include "PlatformMc.h"
#include "McCMD.h"
#include "PUtil.h"
#include "PPrivate.h"
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
static ptime_t g_lastHbTime = 0;
static puint16_t g_sequence;

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
        || code == MC_CMD_RESOURCE_REQUES_ACK
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

_ptag static void messageSend(PrivateCtx_t *ctx, const char *data, puint16_t len, puint16_t sequence)
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
    puint16_t len;
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

_ptag static void sendData(PrivateCtx_t *ctx, int cmd, const char *data, pbool_t needReply)
{
    char *buff;
    puint16_t len;
    char *key = ctx->sessionkey;
    char *iv = key;
    json_item_t *message = json_item_create(JSON_OBJECT, PNULL);
    char *dataStr = PUtilAESEncodeBase64(data, strlen(data), key, iv);

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
    json_item_t *post = json_item_create(JSON_OBJECT, NULL);

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

_ptag static void controlReply(PrivateCtx_t *ctx, puint16_t code, puint32_t result, const char *did, const char *subDid)
{
    char buff[256];
    json_item_t *reply = createMessageHead(did, ctx->recvsn, ptrue);

    json_item_add_subitem(reply, J_CREATE_I(MC_RESULT, result));
    if(subDid)
    {
        json_item_add_subitem(reply, J_CREATE_I(MC_SUBDEV_ID, subDid));
    }
    json_item_parse_to_text(reply, buff);
    json_item_destroy(reply);

    plog("%s", buff);
    sendData(ctx, code, buff, pfalse);
}

_ptag static void controlHandle(PrivateCtx_t *ctx, const char *did, json_item_t *msg)
{
    int i;
    char *cmdName, *cmdParam;
    PMProperty_t *property;
    PPropertyCtrl_t pctrl;
    json_item_t *cmd = json_item_get_subitem_by_name("cmd");

    if(cmd)
    {
        pctrl.did = did;
        pctrl.propertyNum = json_item_get_subitem_count(cmd);
        pctrl.property = malloc(sizeof(PPropertyValue_t) * pctrl.propertyNum);
        memset(pctrl.property, 0, sizeof(PPropertyValue_t) * pctrl.propertyNum);
        for(i = 0; i < pctrl.propertyNum; i++)
        {
            json_item_t *item = json_item_get_subitem_by_index(cmd, i);
            cmdName = J_SUB_VALUE_BY_NAME_S(item, MC_CMDNAME);
            cmdParam = J_SUB_VALUE_BY_NAME_S(item, MC_CMDPARAM);
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
                    pctrl.property[i].value.text = (char *)malloc(strlen(cmdParam) + 1);
                    pctrl.property[i].value.text[0] = '\0';
                    strcpy(pctrl.property[i].value.text, cmdParam);
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
        for(i = 0; i < pctrl.propertyNum; i++)
        {
            if(pctrl.property[i].isText && pctrl.property[i].value.text)
            {
                free(pctrl.property[i].value.text);
            }
        }
        free(pctrl.property);
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

_ptag static void upgradeNoticeHandle(PrivateCtx_t *ctx, json_item_t *msg)
{
    int i, pos = 0;
    char *version;
    PUpgradeNotice_t notice;
    json_item_t *device = json_item_get_subitem_by_name(msg, "device");
    json_item_t *modules = json_item_get_subitem_by_name(msg, "modules");

    notice.infoNum = 0;
    if(modules)
    {
        notice.infoNum = json_item_get_subitem_count(modules);
    }

    if(device)
    {
        version = J_SUB_VALUE_BY_NAME_S(device, MC_VERSION);
        if(version)
        {
            notice.infoNum++;
            notice.info = (PUpgradeNotice_t *)malloc(sizeof(PUpgradeNotice_t) * notice.infoNum);

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
            notice.info[pos + i].name = J_SUB_VALUE_BY_NAME_S(item, "name");
            notice.info[pos + i].url = J_SUB_VALUE_BY_NAME_S(item, "url");
            notice.info[pos + i].version = J_SUB_VALUE_BY_NAME_S(item, MC_VERSION);
            notice.info[pos + i].md5 = J_SUB_VALUE_BY_NAME_S(item, "md5");
        }
    }


    PPrivateEventEmit(ctx, PEVENT_OTA_NOTICE, (void *)&notice);

    controlReply(ctx, PEVENT_OTA_NOTICE - 1, 0, ctx->did, PNULL);
}

_ptag static void resourceManagerHandle()
{
}

_ptag static void parseCtrolMessage(PrivateCtx_t *ctx, int cmd, json_item_t *msg)
{
    char *did;
    PMProperty_t *property;
    ResourceInfo_t *resource;

    plog("cmd = %d", cmd);
    did = J_SUB_VALUE_BY_NAME_S(msg, MC_DEVID);
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
            upgradeNoticeHandle(ctx, msg);
            break;
        case MC_CMD_RESOURCE_MANAGER:
            resourceManagerHandle();
            break;
        default:
            break;
    }
}


_ptag static void subDeviceStatusReport(PrivateCtx_t *ctx, const char *subid, pbool_t online)
{
    char buff[256] = "";
    json_item_t *status = createMessageHead(subid, ++ctx->sendsn, ptrue);

    json_item_add_subitem(status, J_CREATE_S(MC_SUBDEV_ID, subid));
    json_item_add_subitem(status, J_CREATE_I(MC_SUBDEV_ONLINE, online));

    json_item_parse_to_text(status, buff);
    json_item_destroy(status);

    plog("%s", buff);
    sendData(ctx, MC_CMD_SUBONLINE, buff, ptrue);
}

_ptag static void sudDeviceAuthResult(PrivateCtx_t *ctx, const char *did, pbool_t success)
{
    PMSubDevice_t *subDev;

    if(did)
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            if(strcmp(subDev->did, did) == 0)
            {
                if(success)
                {
                    subDev->authStatus = SUBDEV_AUTH_SUCCESS;
                    subDeviceStatusReport(ctx, subDev->did, subDev->online);
                    if(subDev->online)
                    {
                        postProperties(ctx, &subDev->property, subDev->did, ptrue);
                        postResources(ctx, &subDev->resource, subDev->did, ptrue);
                    }
                    postVersion(ctx, subDev->did, subDev->version, subDev->model, &subDev->modules);
                }
                else
                {
                    subDev->authStatus = SUBDEV_AUTH_FAILED;
                    PPrivateEventEmit(ctx, PEVENT_SUBDEV_AUTH_FAILED, (void *)subDev->did);
                }
                break;
            }
        }
    }
}

_ptag void PlatformMcSubDeviceUnbindReport(PrivateCtx_t *ctx, const char *did)
{
    char buff[256] = "";
    json_item_t *unbind = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);
    json_item_add_subitem(unbind, J_CREATE_S(MC_SUBDEV_ID, did));
    json_item_parse_to_text(unbind, buff);
    json_item_destroy(unbind);

    plog("%s", buff);
    sendData(ctx, MC_CMD_SUBUNBIND_REPORT, buff, ptrue);
}

_ptag void PlatformMcSubDeviceOnOffline(PrivateCtx_t *ctx, const char *did, pbool_t online)
{
    PMSubDevice_t *subDev = PNULL;

    PListForeach(&ctx->subDevices, subDev)
    {
        if(strcmp(subDev->did, did) == 0)
        {
            subDev->online = online;
            if(CMServerConnected(ctx))
            {
                subDeviceStatusReport(ctx, subDev->did, online);
            }
            return 0;
        }
    }
    return -1;
}

_ptag static void parseReplyMessage(PrivateCtx_t *ctx, int cmd, json_item_t *msg)
{
//    int len;
    char *tcphost;
    char ip[16] = "";
    puint16_t port;
    char *tmp;
    char *subDid;
    char *authResult;
    puint32_t result;
//    ptime_t utctime;
//    json_item_t *item;

    tmp = J_SUB_VALUE_BY_NAME_S(msg, MC_RESULT);
    result = (puint32_t)strtol(tmp, NULL, 10);

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
            port = (puint16_t)strtol(tmp + 1, NULL, 10);
            CMServerLoginSuccess(ctx, ip, port);
            break;
        case MC_CMD_CONNECT_ACK:
            tmp = J_SUB_VALUE_BY_NAME_S(msg, MC_HEARTBEAT);
            if(tmp)
            {
                ctx->hbIntervel = (puint16_t)strtol(tmp, NULL, 10);
            }
            ctx->lastHbTime = PlatformTime();
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
            break;
        case MC_CMD_DEV_VERSION_ACK:
            break;
        case MC_CMD_UPGRADE_RESULT_ACK:
            break;
        case MC_CMD_RESOURCE_REQUES_ACK:
            break;
        case MC_CMD_RESOURCE_REPORT_ACK:
            break;
        case MC_CMD_TTS_ACK:
            break;
        case MC_CMD_VOICE_CTRL_ACK:
            break;
        default:
            break;
    }
}

_ptag void PlatformMcRecv(PrivateCtx_t *ctx, char *msg, pint16_t len)
{
    char *data, *utcstr;
    puint8_t *contentsStr;
    int contentsLen;
    puint16_t codeNum;
    puint8_t *key, *iv;
    char *head = strstr(msg, MC_PROTO_HEAD);
    json_item_t *pack, *contents, *code;

    plog("%s", msg);

    if(head)
    {
        pack = json_item_parse_from_text(head + 3);
        code = json_item_get_subitem_by_name(pack, MC_CODE);
        codeNum = (puint16_t)strtol(json_item_get_string_value(code), NULL, 10);
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
        plog("data decode:%s", contentsStr);
        json_item_destroy(pack);
        contents = json_item_parse_from_text(contentsStr);
        free(contentsStr);

        /*get utc time*/
        utcstr = J_SUB_VALUE_BY_NAME_S(contents, MC_TIME);
        if(utcstr)
        {
            PPrivateEventEmit(ctx, PEVENT_TIMING_INFO, (puint32_t)strtol(utcstr, NULL, 10));
        }

        json_item_t *seq = json_item_get_subitem_by_name(contents, MC_SEQUENCE);
        puint16_t sequence = (puint16_t)strtol(json_item_get_string_value(seq), NULL, 10);

        if(isAckedCode(codeNum))
        {
            findAndDelMessage(sequence);
            parseReplyMessage(ctx, codeNum, contents);
        }
        else
        {
            ctx->recvsn = sequence;
            parseCtrolMessage(ctx, codeNum, contents);
        }

        json_item_destroy(contents);
    }
}

_ptag static void postProperties(PrivateCtx_t *ctx, PMProperty_t *properties, const char *did, pbool_t force)
{
    PMProperty_t *property;
    PMProperty_t *head = properties;
    json_item_t *post = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *statusSerials = json_item_create(JSON_ARRAY, MC_STATUS_SERIALS);

    PListForeach(head, property)
    {
        if(force || property->changed)
        {
            json_item_t *status = json_item_create(JSON_OBJECT, NULL);
            json_item_add_subitem(status, J_CREATE_I(MC_SERIAL_ID, property->serialID));

            json_item_t *statusSerial = json_item_create(JSON_ARRAY, MC_STATUS_SERIAL);

            json_item_t *node = json_item_create(JSON_OBJECT, NULL);
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

    puint16_t msglen = json_item_eval_parse_text_len(post);
    char *buff = (char *)malloc(msglen + 1);

    json_item_parse_to_text(post, buff);
    json_item_destroy(post);
    plog("%s", buff);

    sendData(ctx, MC_CMD_POST, buff, ptrue);

    free(buff);

}

_ptag static void postResources(PrivateCtx_t *ctx, ResourceInfo_t *resources, const char *did, pbool_t force)
{
    puint16_t i;
    pbool_t changed = false;
    ResourceInfo_t *info;
    ResourceNode_t *nodeInfo;
    ResourceInfo_t *rscHead = resources;

    if(PListFirst(rscHead) == PNULL)
    {
        return;
    }

    json_item_t *post = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_t *rscSerials = json_item_create(JSON_ARRAY, MC_RESOURCE_SERIALS);

    PListForeach(rscHead, info)
    {
        changed = false;
        for(i = 0; i < info->nodeNum; i++)
        {
            if(info->changed[i])
            {
                changed = true;
                break;
            }
        }

        if(changed || force)
        {
            json_item_t *status = json_item_create(JSON_OBJECT, NULL);
            json_item_add_subitem(status, J_CREATE_I(MC_SERIAL_ID, info->serialID));

            json_item_t *rscSerial = json_item_create(JSON_ARRAY, MC_RESOURCE_SERIAL);
            json_item_t *name = json_item_create(JSON_OBJECT, NULL);
            json_item_add_subitem(name, J_CREATE_S(MC_RESOURCE_NAME, info->name));

            json_item_t *rscInfo = json_item_create(JSON_ARRAY, MC_RESOURCE_INFO);
            for(i = 0; i < info->nodeNum; i++)
            {
                if(info->changed[i] || force)
                {
                    json_item_t *node = json_item_create(JSON_OBJECT, NULL);
                    PListForeach(&info->node[i], nodeInfo)
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
                    info->changed[i] = pfalse;
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

    puint16_t msglen = json_item_eval_parse_text_len(post);
    char *buff = (char *)malloc(msglen + 1);

    json_item_parse_to_text(post, buff);
    json_item_destroy(post);
    plog("%s", buff);

    sendData(ctx, MC_CMD_RESOURCE_REPORT, buff, ptrue);
    free(buff);
}

_ptag void postVersion(PrivateCtx_t *ctx, const char *did, const char *version, const char *model, ModulesVersion_t *modules)
{
    json_item_t *ver = createMessageHead(did, ++ctx->sendsn, ptrue);
    json_item_add_subitem(ver, J_CREATE_S(MC_MODEL, model));

    json_item_t *dev = json_item_create(JSON_OBJECT, "device");
    json_item_add_subitem(dev, J_CREATE_S(MC_VERSION, version));

    json_item_t *mod = json_item_create(JSON_ARRAY, "modules");
    ModulesVersion_t *module;
    PListForeach(modules, module)
    {
        json_item_t *node = json_item_create(JSON_OBJECT, NULL);

        json_item_add_subitem(node, J_CREATE_S("name", module->name));
        json_item_add_subitem(node, J_CREATE_S(MC_VERSION, module->version));
        json_item_add_subitem(mod, node);
    }
    json_item_add_subitem(ver, mod);
    json_item_add_subitem(ver, dev);

    puint16_t msglen = json_item_eval_parse_text_len(ver);
    char *buff = (char *)malloc(msglen + 1);

    json_item_parse_to_text(ver, buff);
    json_item_destroy(ver);
    plog("%s", buff);

    sendData(ctx, MC_CMD_DEV_VERSION, buff, ptrue);
    free(buff);
}

_ptag void PlatformMcPostAll(PrivateCtx_t *ctx)
{
    postProperties(ctx, &ctx->properties, ctx->did, ptrue);
    postResources(ctx, &ctx->resources, ctx->did, ptrue);
    postVersion(ctx, ctx->did, ctx->version, ctx->model, &ctx->modules);
}

_ptag void checkPropertiesAndResources(PrivateCtx_t *ctx)
{
    puint16_t i;
    pbool_t changed = false;
    PMProperty_t *property;
    ResourceInfo_t *resource;

    if(CMServerConnected(ctx))//??? clients
    {
        PListForeach(&ctx->properties, property)
        {
            if(property->changed)
            {
                postProperties(ctx, property, ctx->did, pfalse);
                break;
            }
        }

        PListForeach(&ctx->resources, resource)
        {
            for(i = 0; i < resource->nodeNum; i++)
            {
                if(resource->changed[i])
                {
                    changed = ptrue;
                    break;
                }
            }
            if(changed)
            {
                postResources(ctx, resource, ctx->did, pfalse);
                break;
            }
        }
    }
}

_ptag static void doAuthentication(PrivateCtx_t *ctx, PMSubDevice_t *subDev)
{
    char buff[256] = "";
    json_item_t *auth = createMessageHead(ctx->did, ++ctx->sendsn, ptrue);

    json_item_add_subitem(auth, J_CREATE_S(MC_SUBDEV_ID, subDev->did));
    json_item_add_subitem(auth, J_CREATE_S("pin", subDev->pin));

    json_item_parse_to_text(auth, buff);
    json_item_destroy(auth);

    plog("%s", buff);
    sendData(ctx, MC_CMD_SUBAUTH, buff, ptrue);
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

_ptag static void subDevicePropertiesAndResources(PrivateCtx_t *ctx)
{
    puint16_t i;
    pbool_t changed = false;
    PMSubDevice_t *subDev;
    PMProperty_t *property;
    ResourceInfo_t *resource;

    if(CMServerConnected(ctx))
    {
        PListForeach(&ctx->subDevices, subDev)
        {
            changed = false;
            if(subDev->authStatus == SUBDEV_AUTH_SUCCESS || subDev->online)
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
                    for(i = 0; i < resource->nodeNum; i++)
                    {
                        if(resource->changed[i])
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
    char buff[128] = "";
    json_item_t *hb = createMessageHead(ctx->did, ++ctx->sendsn, pfalse);

    json_item_add_subitem(hb, J_CREATE_S(MC_TOKEN, ctx->token));
    json_item_parse_to_text(hb, buff);
    json_item_destroy(hb);
    plog("%s", buff);
    sendData(ctx, MC_CMD_HEARTBEAT, buff, ptrue);
}

_ptag static void heartBeatPoll(PrivateCtx_t *ctx)
{
    if(CMServerConnected(ctx))
    {
        if(PTimeHasPast(ctx->lastHbTime, ctx->hbIntervel * 1000))
        {
            heartbeat(ctx);
            ctx->lastHbTime = PlatformTime();
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
    char buff[256] = "";
    json_item_t *data = json_item_create(JSON_OBJECT, PNULL);

    sprintf(sqnStr, "%d", ctx->sendsn);
    json_item_add_subitem(data, J_CREATE_S(MC_SEQUENCE, sqnStr));
    json_item_add_subitem(data, J_CREATE_S(MC_TOKEN, ctx->token));
    json_item_add_subitem(data, J_CREATE_S(MC_DEVVERSION, ctx->version));
    json_item_add_subitem(data, J_CREATE_S(MC_MODEL, ctx->model));
    json_item_add_subitem(data, J_CREATE_I(MC_TIME, PUtcTime()));
    json_item_parse_to_text(data, buff);
    json_item_destroy(data);
    plog("%s", buff);

    sendData(ctx, MC_CMD_CONNECT, buff, ptrue);
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
}

