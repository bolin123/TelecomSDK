#include "ConnectionManager.h"
#include "Adapter/PAdapter.h"
#include "Adapter/PSocket.h"

static PSocket_t *g_serverSocket = PNULL;
static PSocket_t *g_tcpListenSocket;
static PSocket_t *g_udpListenSocket;
static CMServerStatus_t g_serverStatus = CM_SERVER_STATUS_IDLE;
static ptime_t g_lastServerStatusTime = 0;

_ptag void CMServerSendData(const puint8_t *data, puint32_t len)
{
    PSocketSend(g_serverSocket, data, len);
}

_ptag CMServerStatus_t CMServerStatus(void)
{
    return g_serverStatus;
}

_ptag static void updateServerStatus(CMServerStatus_t status)
{
    if(g_serverStatus != status)
    {
        //event
        g_serverStatus = status;
    }
    g_lastServerStatusTime = PlatformTime();
}

_ptag static void serverConnectCallback(PSocket_t *sock, pbool_t success)
{
    if(g_serverStatus == CM_SERVER_STATUS_LS_CONNECTING)
    {
        updateServerStatus(CM_SERVER_STATUS_LS_CONNECTED);
    }
    else if(g_serverStatus = CM_SERVER_STATUS_CS_CONNECTING)
    {
        updateServerStatus(CM_SERVER_STATUS_CS_CONNECTED);
        //PlatformMcLogin();
    }
}

_ptag static void serverRecvCallback(PSocket_t *sock, const puint8_t *data, pint32_t len)
{
}

_ptag static void serverDisconnectCallback(PSocket_t *sock)
{
    updateServerStatus(CM_SERVER_STATUS_IDLE);
}

_ptag static void appListenCallback(PSocket_t *sock, PSocket_t *newSock)
{
}

_ptag static void discoverRecvCallback(PSocket_t *sock, const puint8_t *data, pint32_t len)
{
}

_ptag static void dnsResolve(const char *host, const char *ip, unsigned char success)
{
    if(success)
    {
        PSocketDestroy(g_serverSocket);
        g_serverSocket = initSocket(PSOCKET_TYPE_TCP);
        PSocketConnect(g_serverSocket, ip, PLATFORM_SERVER_PORT);
    }
}

_ptag static void serverManagerPoll(void)
{
    if(PAdaperIsLinkup())
    {
        if(CM_SERVER_STATUS_IDLE == g_serverStatus)
        {
            if(g_lastServerStatusTime == 0
                || PTimeHasPast(g_lastServerStatusTime, 30000))
            {
                PSocketDnsResolve(PLATFORM_SERVER_URL, dnsResolve);
                updateServerStatus(CM_SERVER_STATUS_LS_CONNECTING);
            }
        }
        else if (CM_SERVER_STATUS_ONLINE == g_serverStatus)
        {
        }
        else
        {
            if(PTimeHasPast(g_lastServerStatusTime, 20000))
            {
                //timeout
                updateServerStatus(CM_SERVER_STATUS_IDLE);
            }
        }

    }
}

void CMConnectionPoll(void)
{

}

static PSocket_t *initSocket(PSocketType_t type)
{
    PSocket_t *socket;

    socket = PSocketCreate(type);
    if(socket)
    {
        socket->connectCallback = serverConnectCallback;
        socket->disconnectCallback = serverDisconnectCallback;
        socket->recvCallback = serverRecvCallback;
        return socket;
    }
    return PNULL;
}

void CMStart(void)
{
    if (g_serverSocket == PNULL)
    {
        g_serverSocket = initSocket(PSOCKET_TYPE_TCP);
    }

    //¾ÖÓòÍøTCP¼àÌý
    if (g_tcpListenSocket == PNULL)
    {
        g_tcpListenSocket = PSocketCreate(PSOCKET_TYPE_TCP);
        g_tcpListenSocket->listenCallback = appListenCallback;
        PSocketStartListen(g_tcpListenSocket, 7681);
    }

    //¾ÖÓòÍøUDP¼àÌý
    if (g_udpListenSocket == PNULL)
    {
        g_udpListenSocket = PSocketCreate(PSOCKET_TYPE_UDP);
        g_udpListenSocket->recvCallback = discoverRecvCallback;
        PSocketStartListen(g_udpListenSocket, 7680);
    }

}

void CMInitialize(void)
{
}

void CMPoll(void)
{
}
