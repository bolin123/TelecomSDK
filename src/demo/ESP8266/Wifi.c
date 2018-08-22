#include "Wifi.h"
#include "user_interface.h"
#include <ctype.h>

static void wifiPromiscuousCb(uint8_t *buf, uint16_t len);
static bool g_inAp;

static WifiScanDoneCb_t g_scanDoneCb;

ROM_FUNC void logApInfo(const char *tip, struct softap_config *cfg)
{
    ulog("%s auth:%d channel:%d max_conn:%d hidden:%d ssid:%s pwd:%s", tip,
           cfg->authmode, cfg->channel, cfg->max_connection, cfg->ssid_hidden,
           cfg->ssid, cfg->password);
}

ROM_FUNC uint8_t WifiSetAp(const char *ssid, const char *passwd, uint8_t ssidHide)
{
    ulog("ssid %s passwd %s", ssid, passwd);
    struct softap_config cfg = {0};

    wifi_set_opmode_current(SOFTAP_MODE);
    if(passwd)
    {
        strcpy((char *)cfg.password, passwd);
        cfg.authmode = AUTH_WPA_WPA2_PSK;
    }
    else
    {
        cfg.authmode = AUTH_OPEN;
    }
    cfg.channel= 1;
    cfg.max_connection = 10;
    cfg.ssid_hidden = ssidHide;
    cfg.beacon_interval = 100;
    strncpy((char *)cfg.ssid, ssid, 32);
    if(!wifi_softap_set_config_current(&cfg))
    {
        ulog("wifi_softap_set_config fail");
    }
    g_inAp = true;
    return 1;
}

ROM_FUNC void WifiCloseAp()
{
    g_inAp = false;
    wifi_set_opmode_current(STATION_MODE);
}

ROM_FUNC uint8_t WifiJoinAp(const char *ssid, const char *passwd, WifiAuthType_t auth)
{
    ulog("ssid:%s %d auth:%d passwd:%s", ssid, strlen(ssid), auth, passwd == NULL ? "NULL" : passwd);

    struct station_config staConfig = {0};
    memcpy(staConfig.ssid, ssid, 32);

    if(auth != WIFI_AUTH_OPEN)
    {
        memcpy(staConfig.password, passwd, 64);
    }
    else
    {
        staConfig.password[0] = '\0';
    }
    staConfig.bssid_set = 0;

    wifi_station_disconnect();
    if(!wifi_station_set_config_current(&staConfig))
    {
        ulog("wifi_station_set_config fail.");
        return 0;
    }
    wifi_station_connect();
    return 1;
}



ROM_FUNC void WifiInitialize()
{
    if(wifi_get_opmode() != STATION_MODE)
    {
        wifi_set_opmode(STATION_MODE);
    }
    wifi_station_set_auto_connect(true);
    ulog("");
}

ROM_FUNC void WifiPoll()
{
}

ROM_FUNC static void wifiScanDone(void *arg, STATUS status)
{
    ulog("%d", status);

    if(status == OK)
    {
        struct bss_info *bss = arg;
        WifiBssInfo_t bssInfoList;
        WifiBssInfo_t *bssInfo;
        VTListInit(&bssInfoList);

        while(bss)
        {
            bssInfo = malloc(sizeof(WifiBssInfo_t));
            memcpy(bssInfo->bssid, bss->bssid, sizeof(bssInfo->bssid));
            memcpy(bssInfo->ssid, bss->ssid, sizeof(bssInfo->ssid));
            bssInfo->hidden = bss->is_hidden;
            bssInfo->auth = (WifiAuthType_t)bss->authmode;
            bssInfo->channel = bss->channel;
            VTListAdd(&bssInfoList, bssInfo);
            bss = bss->next.stqe_next;
        }
        g_scanDoneCb(&bssInfoList);

        //clear
        WifiBssInfo_t *bssInfoLast = NULL;
        VTListForeach(&bssInfoList, bssInfo)
        {
            if(bssInfoLast)
            {
                free(bssInfoLast);
            }
            bssInfoLast = bssInfo;
        }
        if(bssInfoLast)
        {
            free(bssInfoLast);
        }
    }
    else
    {
        g_scanDoneCb(NULL);
    }
}

ROM_FUNC void WifiScan(WifiScanDoneCb_t scanDoneCb)
{
    ulog("");
    g_scanDoneCb = scanDoneCb;

    struct scan_config cfg = {0};
    cfg.show_hidden = 0;
    wifi_station_scan(&cfg, wifiScanDone);
}

ROM_FUNC uint8_t WifiConnected()
{
    return wifi_station_get_connect_status() == STATION_GOT_IP;
}

ROM_FUNC char *WifiGetIp(char *ip)
{
    struct ip_info info;
    wifi_get_ip_info(0, &info);
    sprintf(ip, IPSTR, IP2STR(&info.ip));
    return ip;
}

ROM_FUNC const uint8_t *WifiGetMac()
{
    static uint8_t macAddr[6];
    wifi_get_macaddr(STATION_IF, macAddr);
    return macAddr;
}

ROM_FUNC WifiStatus_t WifiGetStatus()
{
    //ulog("esp wifi status %d", wifi_station_get_connect_status());
    uint8_t status = wifi_station_get_connect_status();
    switch(status)
    {
        case STATION_GOT_IP:
            return WIFI_STATUS_CONNECTED;

        case STATION_WRONG_PASSWORD:
            return WIFI_STATUS_PWD_WRONG;

        case STATION_NO_AP_FOUND:
            return WIFI_STATUS_NO_AP_FOUND;

        case STATION_CONNECTING:
            return WIFI_STATUS_CONNECTING;

        case STATION_CONNECT_FAIL:
            return WIFI_STATUS_CONNECT_FAIL;

        default:
            return WIFI_STATUS_IDLE;
    }
}

ROM_FUNC void WifiReconnect()
{
    wifi_station_disconnect();
    wifi_station_connect();
}

ROM_FUNC char *WifiGetConnectSSID(char *buf)
{
    struct station_config cfg;
    wifi_station_get_config(&cfg);
    memcpy(buf, cfg.ssid, 32);
    buf[32] = '\0';
    return buf;
}

/*******************************************************************************
 * WiFi Sniffer
 ******************************************************************************/
static WifiSnifferCallback_t g_snifferCb;

struct RxControl
{
    signed rssi:8;//表示该包的信号强度
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;//表示该包是否是 11n 的包,0 表示非 11n,非 0 表示 11n
    unsigned legacy_length:12;//如果不是 11n 的包,它表示包的长度
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;//如果是 11n 的包,它表示包的调制编码序列,有效值:0- 76
    unsigned CWB:1;//如果是 11n 的包,它表示是否为 HT40 的包
    unsigned HT_length:16;//如果是 11n 的包,它表示包的长度
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;//如果是 11n 的包,它表示是否为 LDPC 的包
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;//表示该包所在的信道
    unsigned:12;
};

struct LenSeq
{
    u16 len;//包长
    u16 seq;//包的序列号,其中高 12bit 就是序列号,低 4bit 是 Fragment 号(一 般是 0)
    u8 addr3[6];//包中的第 3 个地址
};

struct sniffer_buf
{
    struct RxControl rx_ctrl;
    u8 buf[36];//包含 ieee80211 包头
    u16 cnt;//包的个数
    struct LenSeq lenseq[1];
};

ROM_FUNC static void wifiPromiscuousCb(uint8 *buf, uint16 len)
{
    struct sniffer_buf *sniffer = (struct sniffer_buf *)buf;

    if(len == 128)
    {

    }
    else if(len % 10 == 0)
    {
        uint8_t i;
        for(i = 0; i < sniffer->cnt; i++)
        {
            if(g_snifferCb)
            {
                g_snifferCb(sniffer->buf, sniffer->lenseq[i].len);
            }
        }
    }
}

ROM_FUNC void WifiSnifferStart(WifiSnifferCallback_t cb)
{
    g_snifferCb = cb;
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_disconnect();
    wifi_set_promiscuous_rx_cb(wifiPromiscuousCb);
    wifi_promiscuous_enable(1);
}

ROM_FUNC void WifiSnifferStop()
{
    g_snifferCb = NULL;
    wifi_promiscuous_enable(0);
    wifi_set_opmode_current(STATION_MODE);
}

ROM_FUNC void WifiSnifferSetChannel(uint8_t channel)
{
    wifi_set_channel(channel);
}

ROM_FUNC uint8_t WifiSnifferGetChannel()
{
    return wifi_get_channel();
}

ROM_FUNC void WifiSnifferSetMacFilter(const uint8_t *mac)
{
    wifi_promiscuous_set_mac(mac);
}

ROM_FUNC int8_t WifiGetRSSI()
{
    if(!WifiConnected())
    {
        return -128;
    }

    return wifi_station_get_rssi();
}

