#ifndef WIFI_H
#define WIFI_H
#include "UserTypes.h"
#include "VTList.h"

void WifiInitialize(void);
void WifiPoll(void);

//WiFi��������
typedef enum
{
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK
}WifiAuthType_t;

//WiFi״̬
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

//�ر�APģʽ
void WifiCloseAp(void);

//����AP
bool WifiSetAp(const char *ssid, const char *passwd, uint8_t ssidHide);

//����AP
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

//ɨ��
typedef void (*WifiScanDoneCb_t)(WifiBssInfo_t *bssInfoList);
void WifiScan(WifiScanDoneCb_t scanDoneCb);

//����
void WifiReconnect(void);

//WiFi�Ƿ�����
bool WifiConnected(void);

//��ȡWiFi״̬
WifiStatus_t WifiGetStatus(void);

//��ȡ����AP��RSSI
int8_t WifiGetRSSI(void);

//��ȡIP
char *WifiGetIp(char *ip);

//��ȡ����AP��SSID
char *WifiGetConnectSSID(char *buf);

//��ȡMAC��ַ
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
