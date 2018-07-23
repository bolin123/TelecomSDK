#include "Adapter/PSocket.h"
#include "Util/PList.h"
#include "UserTypes.h"
#include "PlatformUser.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#ifdef __WIN__
#include <WinSock2.h>
#define socklen_t int
#define close closesocket

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#endif

#define PRI(sock) ((struct PSocketPri_st *)((sock)->pri))

/*******************************************************************************
 * 接口适配
 ******************************************************************************/
//#define OPTION_PSOCKET_NO_HTON
typedef struct sockaddr_in vt_sockaddr_in_t;
typedef struct timeval vt_timeval_t;
#define vt_fd_set fd_set
#define vt_socket socket
#define vt_close close
#define vt_bind bind
#define vt_listen listen
#define vt_setsockopt setsockopt
#define vt_accept accept
#define vt_connect connect
#define vt_select select
#define vt_recv recv
#define vt_send send
#define vt_sendto sendto
#define vt_recvfrom recvfrom
#define vt_socket_set_conn_timeout
#define vt_sleep Sleep

bool HalGetHostByName(const char *host, char *ip)
{
    struct hostent *ret;
    ret = gethostbyname(host);

    if(ret)
    {
        struct in_addr n;
        memcpy(&n, ret->h_addr, 4);
        strcpy(ip, inet_ntoa(n));
        return true;
    }

    return false;
}

struct PSocketPri_st
{
    int sd;
    vt_sockaddr_in_t remoteAddr;
    bool isServer : 1;
    PSocket_t *sock;
    PLIST_ENTRY(struct PSocketPri_st);
};

#define XUINT16_C(x)  (x)
#define XUINT32_C(x)  (x ## U)

#define VT_Swap16( X ) \
    ( (uint16_t)( \
        ( ( ( (uint16_t)(X) ) << 8 ) & XUINT16_C( 0xFF00 ) ) | \
        ( ( ( (uint16_t)(X) ) >> 8 ) & XUINT16_C( 0x00FF ) ) ) )

#define VT_Swap32( X ) \
    ( (uint32_t)( \
        ( ( ( (uint32_t)(X) ) << 24 ) & XUINT32_C( 0xFF000000 ) ) | \
        ( ( ( (uint32_t)(X) ) <<  8 ) & XUINT32_C( 0x00FF0000 ) ) | \
        ( ( ( (uint32_t)(X) ) >>  8 ) & XUINT32_C( 0x0000FF00 ) ) | \
        ( ( ( (uint32_t)(X) ) >> 24 ) & XUINT32_C( 0x000000FF ) ) ) )

#ifdef OPTION_PSOCKET_NO_HTON
#define VT_HTONS(n) (n)
#define VT_HTONL(n) (n)
#else
#define VT_HTONS(n) VT_Swap16(n)
#define VT_HTONL(n) VT_Swap32(n)
#endif

/*******************************************************************************
* Global Variable
******************************************************************************/
static struct PSocketPri_st g_sockPriList;

//DNS
static char *g_dnsHost = NULL;
static bool g_dnsResolveStart;
static bool g_dnsResolveEnd;
static bool g_dnsResolveSuccess;
static char g_dsnResolveIP[16];
static PSocketDnsResolveCallback_t g_dnsResolveCb;

//Connect
static bool g_connectStart = false;
static bool g_connectEnd = false;
static PSocket_t *g_connectSock = NULL;
static vt_sockaddr_in_t g_connectAddr;
static bool g_connSockNeedDestroy = false;
static int  g_connectSuccess = -1;

/*******************************************************************************
* Function Declaration
******************************************************************************/
bool HalGetHostByName(const char *host, char *ip);
static void closeSd(int sd);
static int socketSend(int fd, const uint8_t *buf, int len);
static void disconnect(PSocket_t *sock);

/*******************************************************************************
* Function Implement
******************************************************************************/
static int newSd(PSocketType_t type)
{
    int sd;
    if(type == PSOCKET_TYPE_TCP)
    {
        sd = vt_socket(AF_INET, SOCK_STREAM, 0);
        ulog("tcp sd: %d", sd);
    }
    else
    {
        sd = vt_socket(AF_INET, SOCK_DGRAM, 0);
        ulog("udp sd: %d", sd);

        int so_broadcast = 1;
        vt_setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (char *)&so_broadcast, sizeof(so_broadcast));
    }

#ifdef QCA4004
    //容错，QCA4004底层断开连接会自动释放sd，上层判断不到，有可能分配到相同的sd
    int sdTemp;
    PSocketPri_t *pri;
    PListForeach(&g_sockPriList, pri)
    {
        if(pri->sd == sd)
        {
            if(pri->sock->type == PSOCKET_TYPE_TCP)
            {
                sdTemp = vt_socket(AF_INET, SOCK_STREAM, 0);
            }
            else
            {
                sdTemp = vt_socket(AF_INET, SOCK_DGRAM, 0);
            }
            pri->sd = sdTemp;
            break;
        }
    }
#endif

    return sd;
}

static void setSd(PSocket_t *sock, int sd)
{
    closeSd(PRI(sock)->sd);
    PRI(sock)->sd = sd;
}

static void closeSd(int sd)
{
    vt_close(sd);
}

static void addSocket(PSocket_t *sock)
{
    PListAdd(&g_sockPriList, PRI(sock));
}

static void delSocket(PSocket_t *sock)
{
    PListDel(PRI(sock));
}

void PSocketInitialize()
{
    PListInit(&g_sockPriList);
}

static int selectReadSd(int sd)
{
    vt_fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sd, &fds);
    vt_timeval_t tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int ret = vt_select(sd + 1, &fds, NULL, NULL, &tv);

    if(ret <= 0)
    {
        return ret;
    }
    else
    {
        if(FD_ISSET(sd, &fds))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void PSocketAllDisconnect()
{
    ulog("disconnect all");
    struct PSocketPri_st *pri;
    PListForeach(&g_sockPriList, pri)
    {
        if(pri->sock->connected)
        {
            disconnect(pri->sock);
        }
    }

}

void PSocketPoll(void)
{
    vt_sockaddr_in_t addr;
    int addrlen = sizeof(addr);
    PSocket_t *newSock;
    int  newsd;
    char buf[1024];
    int32_t  ret;

    PSocket_t *disSock = NULL;

    struct PSocketPri_st *pri;
    PListForeach(&g_sockPriList, pri)
    {
        //Server
        if(pri->isServer)
        {
            //TCP
            if(pri->sock->type == PSOCKET_TYPE_TCP)
            {
                ret = selectReadSd(pri->sd);
                if(ret > 0)
                {
                    newsd = vt_accept(pri->sd, (void *)&addr, (void *)&addrlen);
                    ulog("accept %d", newsd);

                    newSock = PSocketCreate(PSOCKET_TYPE_TCP);
                    newSock->connected = true;
                    setSd(newSock, newsd);
                    if(pri->sock->listenCallback)
                    {
                        pri->sock->listenCallback(pri->sock, newSock);
                    }
                }
            }
            //UDP
            else
            {
                ret = selectReadSd(pri->sd);
                if(ret > 0)
                {
                    ret = vt_recvfrom(pri->sd, buf, sizeof(buf), 0, (void *)&addr, (void *)&addrlen);
                    if(ret > 0)
                    {
                        pri->remoteAddr = addr;
                        if(pri->sock->recvCallback)
                        {
                            pri->sock->recvCallback(pri->sock, (uint8_t *)buf, (int16_t)ret);
                        }
                    }
                }
            }
        }
        //TCP Client
        else if(pri->sock->type == PSOCKET_TYPE_TCP)
        {
            if(pri->sock->connected)
            {
                ret = selectReadSd(pri->sd);
                if(ret < 0)
                {
                    //断开连接
                    disSock = pri->sock;
                    break;
                }
                else if(ret > 0)
                {
                    ret = (int16_t)vt_recv(pri->sd, buf, sizeof(buf) - 1, 0);
                    if(ret <= 0)
                    {
                        //断开连接
                        disSock = pri->sock;
                        break;
                    }
                    else if(ret > 0)
                    {
#if 0
                        //TODO
                        int i;
                        ulog("recv");
                        for(i = 0; i < ret; i++)
                        {
                            uprintf("%02x ", (buf[i] & 0xff));
                        }
                        uprintf("\n");
#endif
                        if(pri->sock->recvCallback)
                        {
                            buf[ret] = '\0';
                            pri->sock->recvCallback(pri->sock, (uint8_t *)buf, (int16_t)ret);
                        }
                    }
                }
            }
        }
    }

    //处理断开连接
    if(disSock)
    {
        disconnect(disSock);
    }

    //连接完成
    if(g_connectEnd)
    {
        if(g_connectSuccess)
        {
            g_connectSock->connected = true;
            g_connectSock->connectCallback(g_connectSock, 1);
        }
        else
        {
            g_connectSock->connected = false;
            g_connectSock->connectCallback(g_connectSock, 0);
        }
        g_connectEnd = false;
        g_connectSock = NULL;
    }

    //DNS解析完成
    if(g_dnsResolveEnd)
    {
        g_dnsResolveCb(g_dnsHost, g_dsnResolveIP, g_dnsResolveSuccess);

        free(g_dnsHost);
        g_dnsHost = NULL;
        g_dnsResolveEnd = false;
    }
}

static void init(PSocket_t *sock)
{
    sock->pri = (struct PSocketPri_st *)malloc(sizeof(struct PSocketPri_st));
    memset(PRI(sock), 0, sizeof(struct PSocketPri_st));
    PRI(sock)->sd = newSd(sock->type);
    PRI(sock)->sock = sock;
}

static void deinit(PSocket_t *sock)
{
    sock->connectCallback = NULL;
    sock->recvCallback = NULL;
    sock->disconnectCallback = NULL;

    closeSd(PRI(sock)->sd);
    free(PRI(sock));
}

PSocket_t *PSocketCreate(PSocketType_t sockType)
{
    PSocket_t *sock = (PSocket_t *)malloc(sizeof(PSocket_t));
    memset(sock, 0, sizeof(PSocket_t));
    if(sock)
    {
        sock->type = sockType;
        init(sock);
    }
    addSocket(sock);

    ulog("create sd=%d", PRI(sock)->sd);

    return sock;
}

void PSocketDestroy(PSocket_t *sock)
{
    //如果删除的socket是BackPoll里面的正在进行connect的socket，则延迟到连接结果返回后再删除socket
    if(sock == g_connectSock && g_connectSock != NULL && g_connectStart)
    {
        g_connSockNeedDestroy = true;
        ulog("sock %p is connecting to cloud, delay to destroy\n", sock);
        return;
    }

    ulog("destroy sd=%d", PRI(sock)->sd);

    if(sock)
    {
        delSocket(sock);
        deinit(sock);
        free(sock);
    }
}

static int StringToNum(const char *s)
{
    if(strlen(s) == 0)
    {
        return 0;
    }

    bool negative = false;
    const char *end = s;
    const char *p = s + strlen(s) - 1;
    int n = 0;
    int tmpN = 0;
    uint8_t ten = 0;

    //负号
    if(end[0] == '-')
    {
        negative = true;
        end++;
    }

    while(1)
    {
        tmpN = p[0] - '0';
        uint8_t i;
        for(i = 0; i < ten; i++)
        {
            tmpN *= 10;
        }
        n += tmpN;

        if(p == end)
        {
            break;
        }

        ten++;
        p--;
    }
    if(negative)
    {
        return -n;
    }
    return n;
}

static uint32_t _inet_addr(const char *ip)
{
    char part[4];

    uint32_t res = 0;

    int n = 0;
    const char *start = ip;
    const char *p;
    while(n < 4)
    {
        p = strstr(start, ".");
        if(p == NULL)
        {
            p = ip + strlen(ip);
        }
        memset(part, 0, sizeof(part));
        memcpy(part, start, p - start);

        res |= (StringToNum(part) << ((3 - n) * 8));
        start = p + 1;
        n++;
    }

    return VT_HTONL(res);
}


void PSocketConnect(PSocket_t *sock, const char *host, uint16_t port)
{
    ulog("connect %s:%d", host, port);

    if(g_connectStart
        || g_connectEnd)
    {
        if(sock->connectCallback)
        {
            sock->connectCallback(sock, 0);
        }
        return;
    }

    if(sock->connected)
    {
        PSocketDisconnect(sock);
    }

    vt_sockaddr_in_t addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = VT_HTONS(port);
    uint32_t n = _inet_addr(host);
    memcpy(&addr.sin_addr, &n, 4);
    PRI(sock)->remoteAddr = addr;

    g_connectStart = true;
    g_connectSock = sock;
    g_connectAddr = PRI(sock)->remoteAddr;
    vt_socket_set_conn_timeout(15);
}

void PSocketDisconnect(PSocket_t *sock)
{
    int sd;
    sock->connected = false;
    sd = newSd(sock->type);
    setSd(sock, sd);
}

void PSocketStartListen(PSocket_t *sock, uint16_t port)
{
    vt_sockaddr_in_t addr = {0};
#ifdef MW300
    if(sock->type == PSOCKET_TYPE_UDP) //88mw300 打开监听广播包
    {
        netif_add_udp_broadcast_filter(port);
    }
#endif
    addr.sin_family = AF_INET;
    addr.sin_port = VT_HTONS(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    vt_bind(PRI(sock)->sd, (void *)&addr, sizeof(addr));
    PRI(sock)->isServer = true;
    if(sock->type == PSOCKET_TYPE_TCP)
    {
        vt_listen(PRI(sock)->sd, 10);
    }
}

static void disconnect(PSocket_t *sock)
{
    ulog("err, disconnect");
    PSocketDisconnect(sock);
    if(sock->disconnectCallback)
    {
        sock->disconnectCallback(sock);
    }
}

static int socketSend(int fd, const uint8_t *buf, int len)
{
    int nwritten = 0;
    int ret;
    int cnt = 0;

    while(nwritten < len)
    {
        ret = vt_send(fd, (void *)(buf + nwritten), len - nwritten, 0);
        if(ret <= 0)
        {
            ulog("send fail");
            return -1;
        }
        nwritten += ret;
    }

    return len;
}

unsigned int PSocketSend(PSocket_t *sock, const uint8_t *data, unsigned int len)
{
    int ret;
    ustime_t sendTime;
#if 0
    //TODO
    unsigned int i;
    ulog("send");
    for(i = 0; i < len; i++)
    {
        uprintf("%02x ", data[i]);
    }
    uprintf("\n");
#endif
    if(sock->type == PSOCKET_TYPE_TCP)
    {
        if(sock->connected)
        {
            sendTime = PUserTime();
            ret = socketSend(PRI(sock)->sd, (uint8_t *)data, len);
            if(ret < 0)
            {
                disconnect(sock);
            }
        }
    }
    else
    {
        //char text[20];
        //uint16_t port;
        //PSocketGetRemoteAddr(sock, text, &port);
        //ulog("udp send %s:%d", text, port);
        vt_sendto(PRI(sock)->sd, (char *)data, len, 0, (void *)&(PRI(sock)->remoteAddr), sizeof(vt_sockaddr_in_t));
    }
    return 0;
}

unsigned int PSocketSendTo(PSocket_t *sock, const char *ip, uint16_t port, const uint8_t *data, unsigned int len)
{
    vt_sockaddr_in_t addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = VT_HTONS(port);
    uint32_t n = _inet_addr(ip);
    memcpy(&addr.sin_addr, &n, 4);

    PRI(sock)->remoteAddr = addr;
    return PSocketSend(sock, data, len);
}

void PSocketGetRemoteAddr(PSocket_t *sock, char *ip, uint16_t *port)
{
    *port = ntohs(PRI(sock)->remoteAddr.sin_port);

    uint32_t n;
    memcpy(&n, &PRI(sock)->remoteAddr.sin_addr, 4);

    n = VT_HTONL(n);

    sprintf(ip, "%d.%d.%d.%d",
        (n >> 24) & 0xff,
        (n >> 16) & 0xff,
        (n >> 8) & 0xff,
        (n >> 0) & 0xff);
    //return 1;
}

void PSocketDnsResolve(const char *host, PSocketDnsResolveCallback_t callback)
{
    if(g_dnsResolveStart
        || g_dnsResolveEnd)
    {
        callback(host, "0.0.0.0", false);
        return;
    }

    g_dnsHost = malloc(strlen(host) + 1);
    strcpy(g_dnsHost, host);
    g_dnsResolveCb = callback;
    g_dnsResolveStart = true;
}

void PSocketBackPoll(void)
{
    //处理DNS
    if(g_dnsResolveStart)
    {
        g_dnsResolveSuccess = HalGetHostByName(g_dnsHost, g_dsnResolveIP);
        g_dnsResolveEnd = true;
        g_dnsResolveStart = false;
    }

    //处理连接
    if(g_connectStart)
    {
        //ulog("begin conn");
        int ret = vt_connect(PRI(g_connectSock)->sd, (void *)&g_connectAddr, sizeof(g_connectAddr));
        //ulog("conn ret=%d", ret);
        g_connectSuccess = ret >= 0;
        g_connectStart = false;
        //连接过程中出现destroy socket的情况时，连接返回后，直接删除g_connectSock
        if(g_connSockNeedDestroy)
        {
            g_connSockNeedDestroy = false;
            delSocket(g_connectSock);
            deinit(g_connectSock);
            free(g_connectSock);
            g_connectSock = NULL;
        }
        else
        {
            g_connectEnd = true;
        }
    }
}

