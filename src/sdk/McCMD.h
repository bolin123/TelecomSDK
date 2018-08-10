#ifndef MCCMD_H
#define MCCMD_H

/*�豸��������ָ��*/
#define MC_CMD_HEARTBEAT        1000 //������
#define MC_CMD_LOGIN            1002 //�豸��¼��
#define MC_CMD_CONNECT          1004 //�豸���Ӱ�
#define MC_CMD_POST             2006 //�豸״̬�ϱ�
#define MC_CMD_WARNING          2008 //�豸״̬�澯
#define MC_CMD_FAULT            2010 //�豸���Ϸ���
#define MC_CMD_SUBAUTH          2012 //���豸��Ȩ
#define MC_CMD_SUBBIND          2016 //���豸���ϱ�
#define MC_CMD_SUBUNBIND_REPORT 2018 //���豸����ϱ�
#define MC_CMD_SUBONLINE        2020 //���豸����״̬�ϱ�
#define MC_CMD_DEV_VERSION      2022 //�豸�汾�ϱ�
#define MC_CMD_UPGRADE_RESULT   2026 //�豸�汾��������ϱ�
#define MC_CMD_RESOURCE_REPORT  2028 //�豸��Դ�仯�ϱ�
#define MC_CMD_EVENT_REPORT     2032 //�豸��Ϣ�ϱ�
#define MC_CMD_TTS              3000 //�����ϳ�
#define MC_CMD_VOICE_CTRL       3002 //��������

/*������Ӧ��*/
#define MC_CMD_HEARTBEAT_ACK        1001 //����Ӧ��
#define MC_CMD_LOGIN_ACK            1003 //��¼Ӧ��
#define MC_CMD_CONNECT_ACK          1005 //����Ӧ��
#define MC_CMD_POST_ACK             2007 //״̬�ϱ�Ӧ��
#define MC_CMD_WARNING_ACK          2009 //�澯�ϱ�Ӧ��
#define MC_CMD_FAULT_ACK            2011 //�����ϱ�Ӧ��
#define MC_CMD_SUBAUTH_ACK          2013 //���豸��ȨӦ��
#define MC_CMD_SUBBIND_ACK          2017 //���豸��Ӧ��
#define MC_CMD_SUBUNBIND_REPORT_ACK 2019 //���豸���Ӧ��
#define MC_CMD_SUBONLINE_ACK        2021 //���豸����Ӧ��
#define MC_CMD_DEV_VERSION_ACK      2023 //�豸�汾Ӧ��
#define MC_CMD_UPGRADE_RESULT_ACK   2027 //�������Ӧ��
#define MC_CMD_RESOURCE_REPORT_ACK  2029 //��Դ�仯�ϱ�Ӧ��
#define MC_CMD_EVENT_REPORT_ACK     2033 //�¼��ϱ�Ӧ��
#define MC_CMD_TTS_ACK              3001 //�����ϳɽ��
#define MC_CMD_VOICE_CTRL_ACK       3003 //�������ƽ��

/*�����������·�*/
#define MC_CMD_QUERY            2003 //�豸״̬��ѯ
#define MC_CMD_CONTROL          2005 //�豸״̬����
#define MC_CMD_SUBUNBIND        2015 //���豸���
#define MC_CMD_UPGRADE_NOTICE   2025 //�豸�汾����֪ͨ
#define MC_CMD_RESOURCE_MANAGER 2031 //��Դ����

/*�ؼ���*/
#define MC_CODE       "code"
#define MC_DATA       "data"
#define MC_SEQUENCE   "sequence"
#define MC_DEVID      "deviceId"
#define MC_VERSION    "version"
#define MC_TIME       "time"
#define MC_TOKEN      "token"
#define MC_DEVVERSION "devVersion"
#define MC_MODEL      "model"
#define MC_SESSIONKEY "sessionKey"
#define MC_RESULT     "result"
#define MC_TCPHOST    "tcpHost"
#define MC_HEARTBEAT  "heartBeat"
#define MC_AUTHINTERVAL "authInterval"

/*�豸״̬*/
#define MC_STATUS_SERIALS   "statusSerials"
#define MC_STATUS_SERIAL    "statusSerial"
#define MC_SERIAL_ID        "serialId"
#define MC_STATUS_NAME      "statusName"
#define MC_CUR_STATUS_VALUE "curStatusValue"

/*��Դ����*/
#define MC_RESOURCE_SERIALS "resourceSerials"
#define MC_RESOURCE_SERIAL  "resourceSerial"
#define MC_RESOURCE_NAME    "resourceName"
#define MC_RESOURCE_INFO    "resourceInfo"

#define MC_SUBDEV_ID   "subDeviceId"
#define MC_AUTH_RESULT "authResult"
#define MC_SUBDEV_ONLINE "online"

#define MC_EVENT_ID   "eventId"
#define MC_EVENT_NAME "eventName"
#define MC_EVENT_INFO "eventInfo"

#define MC_VOICE_CTRL_PARAM      "ctrlParam"
#define MC_VOICE_CTRL_FORMAT     "format"
#define MC_VOICE_CTRL_RATE       "rate"
#define MC_VOICE_CTRL_CHANNEL    "channel"
#define MC_VOICE_CTRL_VOICEID    "voiceId"
#define MC_VOICE_CTRL_AUDIODATA  "audioData"
#define MC_VOICE_CTRL_EXTENSIONS "extensions"
#define MC_VOICE_CTRL_RESULT     "ctrlResult"

#define MC_TTS_PARAM  "ttsParam"
#define MC_TTS_RESULT "ttsResult"

#define MC_ERROR_CODE "errorCode"
#define MC_ERROR_INFO "errorInfo"
#define MC_ERROR_TIME "errorTime"

#define MC_PROTO_HEAD "CTS"
#define MC_PROTO_END  "\r\n"

#define MC_CMDNAME  "cmdName"
#define MC_CMDPARAM "cmdParam"

#endif
