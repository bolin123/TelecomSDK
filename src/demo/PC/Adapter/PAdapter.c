#include "Adapter/PAdapter.h"
#include "UserTypes.h"
#include <time.h>
#ifdef __WIN__
#include <Windows.h>
#endif

char *PGetCTEI(void)
{
    return "123456789012345";
}

char *PGetMacAddr(char *mac)
{
    //char mac[18] = "";
    NCB ncb;
    typedef struct _ASTAT_
    {
        ADAPTER_STATUS   adapt;
        NAME_BUFFER   NameBuff[30];
    }ASTAT, *PASTAT;

    ASTAT Adapter;
    typedef struct _LANA_ENUM
    {
        UCHAR length;
        UCHAR lana[MAX_LANA];
    }LANA_ENUM;
    LANA_ENUM lana_enum;
    UCHAR uRetCode;
    memset(&ncb, 0, sizeof(ncb));
    memset(&lana_enum, 0, sizeof(lana_enum));

    ncb.ncb_command = NCBENUM;
    ncb.ncb_buffer = (unsigned char *)&lana_enum;
    ncb.ncb_length = sizeof(LANA_ENUM);
    uRetCode = Netbios(&ncb);
    if(uRetCode != NRC_GOODRET)
        return mac;

    for(int lana = 0; lana < lana_enum.length; lana++)
    {
        ncb.ncb_command = NCBRESET;
        ncb.ncb_lana_num = lana_enum.lana[lana];
        uRetCode = Netbios(&ncb);
        if(uRetCode == NRC_GOODRET)
            break;
    }
    if(uRetCode != NRC_GOODRET)
        return mac;

    memset(&ncb, 0, sizeof(ncb));
    ncb.ncb_command = NCBASTAT;
    ncb.ncb_lana_num = lana_enum.lana[0];
    strcpy_s((char*)ncb.ncb_callname, 5, "*");
    ncb.ncb_buffer = (unsigned char *)&Adapter;
    ncb.ncb_length = sizeof(Adapter);
    uRetCode = Netbios(&ncb);
    if(uRetCode != NRC_GOODRET)
        return mac;
    sprintf_s(mac, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
        Adapter.adapt.adapter_address[0],
        Adapter.adapt.adapter_address[1],
        Adapter.adapt.adapter_address[2],
        Adapter.adapt.adapter_address[3],
        Adapter.adapt.adapter_address[4],
        Adapter.adapt.adapter_address[5]
    );
    return mac;
}

char *PGetIpAddr(char *ip)
{
    // 获得本机主机名
    char hostname[255] = { 0 };

    gethostname(hostname, sizeof(hostname));

    struct hostent FAR* lpHostEnt = gethostbyname(hostname);

    if(lpHostEnt == NULL)
    {
        return NULL;
    }

    // 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
    LPSTR lpAddr = lpHostEnt->h_addr_list[0];

    // 将IP地址转化成字符串形式
    struct in_addr inAddr;
    memmove(&inAddr, lpAddr, 4);
    strcpy(ip, inet_ntoa(inAddr));
    return ip;
}

unsigned char PIsLinkup(void)
{
    return true;
}

unsigned int PUtcTime(void)
{
    return (uint32_t)time(NULL);
}

unsigned int PlatformTime(void)
{
    return (uint32_t)GetTickCount();
}

