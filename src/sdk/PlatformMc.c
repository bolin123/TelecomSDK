#include "PlatformMc.h"
#include "PUtil.h"
#include "Util/json.h"

#define PLATFORM_MC_CMD_HEARTBEAT 1000 //������
#define PLATFORM_MC_CMD_LOGIN 1002 //�豸��¼��
#define PLATFORM_MC_CMD_CONNECT 1004 //�豸���Ӱ�
#define PLATFORM_MC_CMD_QUERY 2003 //�豸״̬��ѯ
#define PLATFORM_MC_CMD_CONTROL 2005 //�豸״̬����
#define PLATFORM_MC_CMD_POST 2006 //�豸״̬�ϱ�
#define PLATFORM_MC_CMD_WARNING 2008 //�豸״̬�澯
#define PLATFORM_MC_CMD_FAULT 2010 //�豸���Ϸ���
#define PLATFORM_MC_CMD_SUBAUTH 2012 //���豸��Ȩ
#define PLATFORM_MC_CMD_SUBUNBIND 2015 //���豸���
#define PLATFORM_MC_CMD_SUBBIND 2016 //���豸���ϱ�
#define PLATFORM_MC_CMD_SUBUNBIND_REPORT 2018 //���豸����ϱ�
#define PLATFORM_MC_CMD_SUBONLINE 2020 //���豸����״̬�ϱ�
#define PLATFORM_MC_CMD_DEV_VERSION 2022 //�豸�汾�ϱ�
#define PLATFORM_MC_CMD_UPGRADE_NOTICE 2025 //�豸�汾����֪ͨ
#define PLATFORM_MC_CMD_UPGRADE_RESULT 2026 //�豸�汾��������ϱ�
#define PLATFORM_MC_CMD_TTS 3000 //�����ϳ�
#define PLATFORM_MC_CMD_VOICE_CTRL 3002 //��������

#define MC_CODE "code"
#define MC_DATA "data"
#define MC_SEQUENCE "sequence"
#define MC_DEVID "deviceId"
#define MC_VERSION "version"
#define MC_TIME "time"
#define MC_TOKEN "token"
#define MC_DEVVERSION "devVersion"
#define MC_MODEL "model"
#define MC_DEVID "deviceId"

_ptag static void sendDataWithID(int cmd, const char *id, const char *data)
{
    json_item_t *message = json_item_create(JSON_OBJECT, PNULL);
    json_item_add_subitem(message, J_CREATE_I(MC_CODE, cmd));
    json_item_add_subitem(message, J_CREATE_S(MC_DEVID, id));
    json_item_add_subitem(message, J_CREATE_S(MC_DATA, data));
 

    //lowsend
    json_item_destroy(message);
}

_ptag static void sendDataWithToken(int cmd, const char *token, const char *data)
{
    json_item_t *message = json_item_create(JSON_OBJECT, PNULL);
    char *dataStr = PUtilAESEncodeBase64(data, strlen(data), "PINd6af6c3134308", "941c2d70a830c950");

    json_item_add_subitem(message, J_CREATE_I(MC_CODE, cmd));
    json_item_add_subitem(message, J_CREATE_S(MC_TOKEN, token));
    json_item_add_subitem(message, J_CREATE_S(MC_DATA, dataStr));
    free(dataStr);

    //lowsend
    json_item_destroy(message);
}

_ptag void PlatformMcLogin(const char *devid, puint16_t sequence)
{
    char buff[256] = "";
    char sqnStr[16] = "";
    json_item_t *data = json_item_create(JSON_OBJECT, PNULL);

    sprintf(sqnStr, "%d", sequence);

    json_item_add_subitem(data, J_CREATE_S(MC_DEVID, devid));
    json_item_add_subitem(data, J_CREATE_S(MC_SEQUENCE, sqnStr));
    json_item_add_subitem(data, J_CREATE_I(MC_TIME, SysUtcTime()));
    json_item_add_subitem(data, J_CREATE_S(MC_VERSION, PLATFORM_PROTOCOL_VERSION));
    json_item_parse_to_text(data, buff);
    json_item_destroy(data);
    plog("%s", buff);
    
    char *dataStr = PUtilAESEncodeBase64(buff, strlen(buff), "PINd6af6c3134308", "941c2d70a830c950");
    sendDataWithID(PLATFORM_MC_CMD_LOGIN, devid, dataStr);
    free(dataStr);
}

_ptag void PlatformMcOnline()
{
    json_item_t *data = json_item_create(JSON_OBJECT, PNULL);
    json_item_add_subitem(MC_SEQUENCE, "");
    json_item_add_subitem(MC_TOKEN, "");
    json_item_add_subitem(MC_DEVVERSION, "");
    json_item_add_subitem(MC_MODEL, "");
    json_item_add_subitem(MC_TIME, "");

    sendDataWithToken();
}


_ptag void PlatformMcInit(void)
{
}
