#ifndef MCCMD_H
#define MCCMD_H

/*设备主动发送指令*/
#define MC_CMD_HEARTBEAT 1000 //心跳包
#define MC_CMD_LOGIN 1002 //设备登录包
#define MC_CMD_CONNECT 1004 //设备连接包
#define MC_CMD_POST 2006 //设备状态上报
#define MC_CMD_WARNING 2008 //设备状态告警
#define MC_CMD_FAULT 2010 //设备故障反馈
#define MC_CMD_SUBAUTH 2012 //子设备鉴权
#define MC_CMD_SUBBIND 2016 //子设备绑定上报
#define MC_CMD_SUBUNBIND_REPORT 2018 //子设备解绑上报
#define MC_CMD_SUBONLINE 2020 //子设备在线状态上报
#define MC_CMD_DEV_VERSION 2022 //设备版本上报
#define MC_CMD_UPGRADE_RESULT 2026 //设备版本升级结果上报
#define MC_CMD_RESOURCE_REPORT 2028 //
#define MC_CMD_EVENT_REPORT 2032
#define MC_CMD_TTS 3000 //语音合成
#define MC_CMD_VOICE_CTRL 3002 //语音控制

/*服务器应答*/
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

/*服务器主动下发*/
#define MC_CMD_QUERY 2003 //设备状态查询
#define MC_CMD_CONTROL 2005 //设备状态控制
#define MC_CMD_SUBUNBIND 2015 //子设备解绑
#define MC_CMD_UPGRADE_NOTICE 2025 //设备版本升级通知
#define MC_CMD_RESOURCE_MANAGER 2031

/*关键字*/
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

/*设备状态*/
#define MC_STATUS_SERIALS   "statusSerials"
#define MC_STATUS_SERIAL    "statusSerial"
#define MC_SERIAL_ID        "serialId"
#define MC_STATUS_NAME      "statusName"
#define MC_CUR_STATUS_VALUE "curStatusValue"

/*资源管理*/
#define MC_RESOURCE_SERIALS "resourceSerials"
#define MC_RESOURCE_SERIAL  "resourceSerial"
#define MC_RESOURCE_NAME    "resourceName"
#define MC_RESOURCE_INFO    "resourceInfo"

#define MC_PROTO_HEAD "CTS"

#endif
