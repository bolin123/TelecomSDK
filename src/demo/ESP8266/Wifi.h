#ifndef WIFI_H
#define WIFI_H
#include "UserTypes.h"
#include "VTList.h"

void WifiInitialize(void);
void WifiPoll(void);

//WiFi加密类型
typedef enum
{
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK
}WifiAuthType_t;

//WiFi状态
typedef enum
{
    WIFI_STATUS_CONNECTED = 0,
    WIFI_STATUS_IDLE = 1,
    WIFI_STATUS_CONNECTING = 2,
    WIFI_STATUS_GETTING_IP = 3,
    WIFI_STATUS_PWD_WRONG = 4,
    WIFI_STATUS_NO_AP_FOUND = 5,
    WIFI_STATUS_CONNECT_FAIL = 6,
}WifiStatus_t;

//关闭AP模式
void WifiCloseAp(void);

//开启AP
bool WifiSetAp(const char *ssid, const char *passwd, uint8_t ssidHide);

//连接AP
bool WifiJoinAp(const char *ssid, const char *passwd, WifiAuthType_t auth);

//BSS
typedef struct WifiBssInfo_st
{
    char ssid[33];
    uint8_t channel;
    bool hidden;
    uint8_t bssid[6];
    WifiAuthType_t auth;
    VTLIST_ENTRY(struct WifiBssInfo_st);
}WifiBssInfo_t;

//扫描
typedef void (*WifiScanDoneCb_t)(WifiBssInfo_t *bssInfoList);
void WifiScan(WifiScanDoneCb_t scanDoneCb);

//重连
void WifiReconnect(void);

//WiFi是否连接
bool WifiConnected(void);

//获取WiFi状态
WifiStatus_t WifiGetStatus(void);

//获取连接AP的RSSI
int8_t WifiGetRSSI(void);

//获取IP
char *WifiGetIp(char *ip);

//获取连接AP的SSID
char *WifiGetConnectSSID(char *buf);

//获取MAC地址
const uint8_t *WifiGetMac(void);

//WiFi Sniffer


typedef enum
{
    WIFI_SNIFFER_STATUS_NOR,
    WIFI_SNIFFER_STATUS_LOCK,
}WifiSnifferStatus_t;

typedef WifiSnifferStatus_t (*WifiSnifferCallback_t)(const uint8_t *data, uint16_t len);
void WifiSnifferStart(WifiSnifferCallback_t cb);
void WifiSnifferStop(void);
void WifiSnifferSetMacFilter(const uint8_t *mac);
void WifiSnifferSetChannel(uint8_t channel);
uint8_t WifiSnifferGetChannel(void);

#endif // WIFI_H
