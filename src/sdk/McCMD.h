#ifndef MCCMD_H
#define MCCMD_H

/*�豸��������ָ��*/
#define MC_CMD_HEARTBEAT 1000 //������
#define MC_CMD_LOGIN 1002 //�豸��¼��
#define MC_CMD_CONNECT 1004 //�豸���Ӱ�
#define MC_CMD_POST 2006 //�豸״̬�ϱ�
#define MC_CMD_WARNING 2008 //�豸״̬�澯
#define MC_CMD_FAULT 2010 //�豸���Ϸ���
#define MC_CMD_SUBAUTH 2012 //���豸��Ȩ
#define MC_CMD_SUBBIND 2016 //���豸���ϱ�
#define MC_CMD_SUBUNBIND_REPORT 2018 //���豸����ϱ�
#define MC_CMD_SUBONLINE 2020 //���豸����״̬�ϱ�
#define MC_CMD_DEV_VERSION 2022 //�豸�汾�ϱ�
#define MC_CMD_UPGRADE_RESULT 2026 //�豸�汾��������ϱ�
#define MC_CMD_RESOURCE_REPORT 2028 //
#define MC_CMD_EVENT_REPORT 2032
#define MC_CMD_TTS 3000 //�����ϳ�
#define MC_CMD_VOICE_CTRL 3002 //��������

/*������Ӧ��*/
#define MC_CMD_HEARTBEAT_ACK 1001
#define MC_CMD_LOGIN_ACK 1003
#define MC_CMD_CONNECT_ACK 1005
#define MC_CMD_POST_ACK 2007
#define MC_CMD_WARNING_ACK 2009
#define MC_CMD_FAULT_ACK 2011
#define MC_CMD_SUBAUTH_ACK 2013
#define MC_CMD_SUBBIND_ACK 2017
#define MC_CMD_SUBUNBIND_REPORT_ACK 2019
#define MC_CMD_SUBONLINE_ACK 2021
#define MC_CMD_DEV_VERSION_ACK 2023
#define MC_CMD_UPGRADE_RESULT_ACK 2027
#define MC_CMD_RESOURCE_REQUES_ACK 2029 //
#define MC_CMD_RESOURCE_REPORT_ACK 2033
#define MC_CMD_TTS_ACK 3001
#define MC_CMD_VOICE_CTRL_ACK 3003

/*�����������·�*/
#define MC_CMD_QUERY 2003 //�豸״̬��ѯ
#define MC_CMD_CONTROL 2005 //�豸״̬����
#define MC_CMD_SUBUNBIND 2015 //���豸���
#define MC_CMD_UPGRADE_NOTICE 2025 //�豸�汾����֪ͨ
#define MC_CMD_RESOURCE_MANAGER 2031

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

#define MC_PROTO_HEAD "CTS"

#endif
