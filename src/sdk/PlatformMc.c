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

_ptag pbool_t isAckedCode(puint16_t code)
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
                // TODO:event, reconnect
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

_ptag void PlatformMcDisconnectHandle(PrivateCtx_t *ctx)
{
    PMessageList_t *msg;

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

}

_ptag static void parseCtrolMessage(int cmd, json_item_t *msg)
{
     plog("cmd = %d", cmd);
}

_ptag static void parseReplyMessage(PrivateCtx_t *ctx, int cmd, json_item_t *msg)
{
//    int len;
    char *tcphost;
    char ip[16] = "";
    puint16_t port;
    char *tmp;
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

    plog("cmd = %d, result = %d", cmd, result);
    switch(cmd)
    {
        case MC_CMD_HEARTBEAT_ACK:
            break;
        case MC_CMD_LOGIN_ACK:
            // TODO:: event login
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
            // TODO: event connected
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
            //event (ptime_t)strtol(utcstr, NULL, 10);
            // TODO:utc time
        }

        if(isAckedCode(codeNum))
        {
            json_item_t *seq = json_item_get_subitem_by_name(contents, MC_SEQUENCE);
            findAndDelMessage((puint16_t)strtol(json_item_get_string_value(seq), NULL, 10));
            parseReplyMessage(ctx, codeNum, contents);
        }
        else
        {
            parseCtrolMessage(codeNum, contents);
        }

        json_item_destroy(contents);
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

_ptag static void sendData(PrivateCtx_t *ctx, int cmd, const char *data)
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

        messageSend(ctx, buff, strlen(buff), ctx->sendsn);

        free(buff);
    }

}

_ptag static json_item_t *createMessageHead(PrivateCtx_t *ctx, pbool_t needDid)
{
    char sqnStr[16] = "";
    json_item_t *post = json_item_create(JSON_OBJECT, NULL);

    sprintf(sqnStr, "%d", ++ctx->sendsn);
    json_item_add_subitem(post, J_CREATE_S(MC_SEQUENCE, sqnStr));
    if(needDid)
    {
        json_item_add_subitem(post, J_CREATE_S(MC_DEVID, ctx->did));
    }
    json_item_add_subitem(post, J_CREATE_I(MC_TIME, PUtcTime()));
    return post;
}

_ptag static void postProperties(PrivateCtx_t *ctx, pbool_t force)
{
    PMProperty_t *property;
    PMProperty_t *head = &ctx->properties;
    json_item_t *post = createMessageHead(ctx, ptrue);
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

    sendData(ctx, MC_CMD_POST, buff);

    free(buff);

}

_ptag static void postResources(PrivateCtx_t *ctx, pbool_t force)
{
    puint16_t i;
    pbool_t changed = false;
    ResourceInfo_t *info;
    ResourceNode_t *nodeInfo;
    ResourceInfo_t *rscHead = &ctx->resources;

    if(PListFirst(rscHead) == PNULL)
    {
        return;
    }

    json_item_t *post = createMessageHead(ctx, ptrue);
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

    sendData(ctx, MC_CMD_RESOURCE_REPORT, buff);
    free(buff);
}

_ptag void PlatformMcPostAll(PrivateCtx_t *ctx)
{
    postProperties(ctx, ptrue);
    postResources(ctx, ptrue);
}

_ptag void checkPropertiesAndResources(PrivateCtx_t *ctx)
{
    puint16_t i;
    pbool_t changed = false;
    PMProperty_t *property;
    ResourceInfo_t *resource;

    if(CMIsOnlineOrClientConnect(ctx))
    {
        PListForeach(&ctx->properties, property)
        {
            if(property->changed)
            {
                postProperties(ctx, pfalse);
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
                postResources(ctx, pfalse);
                break;
            }
        }
    }
}

_ptag static void heartbeat(PrivateCtx_t *ctx)
{
    char buff[128] = "";
    json_item_t *hb = createMessageHead(ctx, pfalse);

    json_item_add_subitem(hb, J_CREATE_S(MC_TOKEN, ctx->token));
    json_item_parse_to_text(hb, buff);
    json_item_destroy(hb);
    plog("%s", buff);
    sendData(ctx, MC_CMD_HEARTBEAT, buff);
}

_ptag static void heartBeatPoll(PrivateCtx_t *ctx)
{
    if(CMServerStatus(ctx) == CM_SERVER_STATUS_ONLINE)
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
    json_item_t *data = createMessageHead(ctx, ptrue);

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

    sendData(ctx, MC_CMD_CONNECT, buff);
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
}

