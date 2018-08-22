#include "Adapter/PSocket.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/ip.h"
#include "lwip/init.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/dhcp.h"
#include "espconn.h"

typedef struct Pri_st
{
    struct tcp_pcb *tcp;
    struct udp_pcb *udp;
    uint8_t *waitSendBuf;
    uint16_t waitSendLen;
    ip_addr_t remoteAddr;
    uint16_t remotePort;
    bool connecting;
    bool sending;
}Pri_t;

static int32_t lowSend(PSocket_t *sock, const uint8_t *data, int32_t len);
static void  errCallabck(void *arg, err_t err);
static void disconnectCallback(void *arg);
static err_t recvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

ROM_FUNC err_t sentCallback(void *arg, struct tcp_pcb *tpcb,
                             uint16_t len)
{
    ulog("");

    PSocket_t *sock = arg;
    ((Pri_t *)sock->pri)->sending = false;

    if(((Pri_t *)sock->pri)->waitSendLen)
    {
        lowSend(sock, ((Pri_t *)sock->pri)->waitSendBuf, ((Pri_t *)sock->pri)->waitSendLen);
    }
    return 0;
}

ROM_FUNC static void setupTcp(PSocket_t *sock, struct tcp_pcb *tcp)
{
    ((Pri_t *)sock->pri)->tcp = tcp;
    tcp_err(tcp, errCallabck);
    tcp_arg(tcp, sock);
    tcp_sent(tcp, sentCallback);
}

ROM_FUNC static void setConn(PSocket_t *sock, struct tcp_pcb *tcp)
{
    tcp_close(((Pri_t *)sock->pri)->tcp);
    setupTcp(sock, tcp);
}

ROM_FUNC static void udpRecvCallback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                          ip_addr_t *addr, uint16_t port)
{
    ulog("");
    PSocket_t *sock = arg;
    memcpy(&((Pri_t *)sock->pri)->remoteAddr, addr, sizeof(ip_addr_t));
    ((Pri_t *)sock->pri)->remotePort = port;
    if(sock->recvCallback)
    {
        sock->recvCallback(sock, p->payload, p->tot_len);
    }
    pbuf_free(p);
}

ROM_FUNC static void init(PSocket_t *sock)
{
    sock->pri = (Pri_t *)malloc(sizeof(Pri_t));
    memset(((Pri_t *)sock->pri), 0, sizeof(Pri_t));

    if(sock->type == PSOCKET_TYPE_TCP)
    {
        setupTcp(sock, tcp_new());
    }
    else
    {
        ulog("create udp");
        ((Pri_t *)sock->pri)->udp = udp_new();
        udp_recv(((Pri_t *)sock->pri)->udp, udpRecvCallback, sock);
    }
}

ROM_FUNC static void deinit(PSocket_t *sock)
{
    ulog("%d", sock->connected);

    if(((Pri_t *)sock->pri)->tcp)
    {
        tcp_recv(((Pri_t *)sock->pri)->tcp, NULL);
        tcp_err(((Pri_t *)sock->pri)->tcp, NULL);
        tcp_close(((Pri_t *)sock->pri)->tcp);
    }

    if(((Pri_t *)sock->pri)->udp)
    {
        udp_remove(((Pri_t *)sock->pri)->udp);
    }

    if(((Pri_t *)sock->pri)->waitSendBuf)
    {
        free(((Pri_t *)sock->pri)->waitSendBuf);
    }

    free(((Pri_t *)sock->pri));
}

ROM_FUNC PSocket_t *PSocketCreate(PSocketType_t sockType)
{
    PSocket_t *sock = (PSocket_t *)malloc(sizeof(PSocket_t));
    memset(sock, 0, sizeof(PSocket_t));
    if(sock)
    {
        ulog("");
        sock->type = sockType;
        init(sock);
    }
    return sock;
}

ROM_FUNC void PSocketDestroy(PSocket_t *sock)
{
    ulog("%p", sock);
    if(sock)
    {
        deinit(sock);
        free(sock);
    }
}

ROM_FUNC static err_t recvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    PSocket_t *sock = arg;
    //ulog("%p", sock);

    if(!p)
    {
        ulog("recv 0");
        if(sock->disconnectCallback)
        {
            sock->disconnectCallback(sock);
        }
        return 0;
    }

    if(sock->recvCallback)
    {
        sock->recvCallback(sock, p->payload, p->tot_len);
    }
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return 0;
}

ROM_FUNC static err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    PSocket_t *sock = arg;
    ulog("err:%d", err);
    if(err == ERR_OK)
    {
        ((Pri_t *)sock->pri)->connecting = 0;
        sock->connected = 1;
        sock->connectCallback(sock, true);
        tcp_recv(tpcb, recvCallback);
    }
    else
    {
        ((Pri_t *)sock->pri)->connecting = 0;
        sock->connectCallback(sock, false);
    }
    return 0;
}

ROM_FUNC static err_t listenCallback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    PSocket_t *sock = arg;

    ulog("");

    if(!sock->listenCallback)
    {
        tcp_close(newpcb);
        return 0;
    }

    ulog("");
    PSocket_t *newSock = PSocketCreate(PSOCKET_TYPE_TCP);
    setConn(newSock, newpcb);
    newSock->connected = 1;
    tcp_recv(newpcb, recvCallback);
    sock->listenCallback(sock, newSock);
    return 0;
}

ROM_FUNC static void  errCallabck(void *arg, err_t err)
{
    PSocket_t *sock = arg;
    sock->connected = 0;

    ulog("%d", err);

    setupTcp(sock, tcp_new());
    if(((Pri_t *)sock->pri)->connecting)
    {
        ulog("connect fail.");
        ((Pri_t *)sock->pri)->connecting = 0;
        if(sock->connectCallback)
        {
            sock->connectCallback(sock, false);
        }
    }
    else
    {
        ulog("disconnect.");
        if(sock->disconnectCallback)
        {
            sock->disconnectCallback(sock);
        }
    }
}

ROM_FUNC void PSocketConnect(PSocket_t *sock, const char *host, uint16_t port)
{
    ulog("%s:%d", host, port);
    if(sock->type == PSOCKET_TYPE_TCP)
    {
        if(sock->connected)
        {
            PSocketDisconnect(sock);
        }

        ip_addr_t addr;
        addr.addr = ipaddr_addr(host);
        ulog("xx:%s", ipaddr_ntoa(&addr));
        ((Pri_t *)sock->pri)->connecting = 1;
        tcp_connect(((Pri_t *)sock->pri)->tcp, &addr, port, connectCallback);
    }
}

ROM_FUNC void PSocketDisconnect(PSocket_t *sock)
{
    ulog("");
    if(((Pri_t *)sock->pri)->tcp)
    {
        tcp_recv(((Pri_t *)sock->pri)->tcp, NULL);
        tcp_err(((Pri_t *)sock->pri)->tcp, NULL);
        tcp_close(((Pri_t *)sock->pri)->tcp);
        setupTcp(sock, tcp_new());
    }
}

ROM_FUNC void PSocketStartListen(PSocket_t *sock, uint16_t port)
{
    if(sock->type == PSOCKET_TYPE_TCP)
    {
        tcp_bind(((Pri_t *)sock->pri)->tcp, IP_ADDR_ANY, port);
        ((Pri_t *)sock->pri)->tcp = tcp_listen(((Pri_t *)sock->pri)->tcp);
        tcp_arg(((Pri_t *)sock->pri)->tcp, sock);
        tcp_accept(((Pri_t *)sock->pri)->tcp, listenCallback);
    }
    else
    {
        udp_bind(((Pri_t *)sock->pri)->udp, IP_ADDR_ANY, port);
    }
}

ROM_FUNC static int32_t lowSend(PSocket_t *sock, const uint8_t *data, int32_t len)
{
    uint16_t sendLen = len;
    uint16_t bufLen = tcp_sndbuf(((Pri_t *)sock->pri)->tcp);

    if(bufLen < sendLen)
    {
        sendLen = bufLen;
    }

    if(sendLen > (2 * ((Pri_t *)sock->pri)->tcp->mss))
    {
        sendLen = 2 * ((Pri_t *)sock->pri)->tcp->mss;
    }

    err_t err;
    do
    {
        err = tcp_write(((Pri_t *)sock->pri)->tcp, data, sendLen, 0);
        if (err == ERR_MEM)
        {
            sendLen >>= 1;
        }
    }while(err == ERR_MEM && sendLen > 1);

    if(err == ERR_OK)
    {
        ((Pri_t *)sock->pri)->sending = true;

        uint16_t lastLen = len - sendLen;
        uint8_t *tmpBuf = NULL;
        if(lastLen)
        {
            tmpBuf = malloc(lastLen);
            memcpy(tmpBuf, data + sendLen, lastLen);
        }
        if(((Pri_t *)sock->pri)->waitSendBuf)
        {
            free(((Pri_t *)sock->pri)->waitSendBuf);
        }
        ((Pri_t *)sock->pri)->waitSendBuf = tmpBuf;
        ((Pri_t *)sock->pri)->waitSendLen = lastLen;

        tcp_output(((Pri_t *)sock->pri)->tcp);
    }
    else
    {
        ulog("send error");
        //close
        if(((Pri_t *)sock->pri)->waitSendBuf)
        {
            free(((Pri_t *)sock->pri)->waitSendBuf);
        }
        ((Pri_t *)sock->pri)->waitSendBuf = NULL;
        ((Pri_t *)sock->pri)->waitSendLen = 0;
        return -1;
    }
    //ulog("x");
    return 0;
}

ROM_FUNC int32_t PSocketSend(PSocket_t *sock, const uint8_t *data, int32_t len)
{
    //ulog("e");
    if(((Pri_t *)sock->pri)->tcp)
    {
        if(((Pri_t *)sock->pri)->sending)
        {
            uint8_t *new = malloc(((Pri_t *)sock->pri)->waitSendLen + len);
            if(new  == NULL)
            {
                ulog("no mem");
                return 0;
            }

            //copy old data
            memcpy(new, ((Pri_t *)sock->pri)->waitSendBuf, ((Pri_t *)sock->pri)->waitSendLen);
            free(((Pri_t *)sock->pri)->waitSendBuf);
            ((Pri_t *)sock->pri)->waitSendBuf = new;

            //append data
            memcpy(((Pri_t *)sock->pri)->waitSendBuf + ((Pri_t *)sock->pri)->waitSendLen, data, len);
            ((Pri_t *)sock->pri)->waitSendLen += len;

            ulog("append to waitSendBuf.");
            return 0;
        }

        return lowSend(sock, data, len);
    }
    else
    {
        struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
        pbuf_take(buf, data, len);
        udp_sendto(((Pri_t *)sock->pri)->udp, buf, &((Pri_t *)sock->pri)->remoteAddr, ((Pri_t *)sock->pri)->remotePort);
        pbuf_free(buf);
        return 0;
    }
}

ROM_FUNC int32_t PSocketSendTo(PSocket_t *sock, const char *ip, uint16_t port, const uint8_t *data, int32_t len)
{
    if(!((Pri_t *)sock->pri)->udp)
    {
        return -1;
    }

    ip_addr_t addr;
    addr.addr = ipaddr_addr(ip);
    struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    pbuf_take(buf, data, len);
    udp_sendto(((Pri_t *)sock->pri)->udp, buf, &addr, port);
    pbuf_free(buf);
    return 0;
}

ROM_FUNC static void dnsResolveCallback(const char *host, ip_addr_t *ip, void *arg)
{
    char ipStr[20];
    PSocketDnsResolveCallback_t cb = arg;

    if(ip == NULL)
    {
        ulog("host resolve dns fail %s", host);
        cb(host, "0.0.0.0", false);
        return;
    }

    sprintf(ipStr, IPSTR, IP2STR(ip));
    cb(host, ipStr, true);
}

ROM_FUNC void PSocketDnsResolve(const char *host, PSocketDnsResolveCallback_t callback)
{
    static ip_addr_t hostip; //must be static
    dns_gethostbyname(host, &hostip, dnsResolveCallback, callback);
}

ROM_FUNC void PSocketInitialize()
{
    espconn_tcp_set_max_con(6);
}

ROM_FUNC void PSocketPoll()
{

}

