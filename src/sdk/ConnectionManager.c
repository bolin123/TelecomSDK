#include "ConnectionManager.h"
#include "Adapter/PAdapter.h"
#include "Adapter/PSocket.h"

static PSocket_t *g_serverSocket = PNULL;
static PSocket_t *g_tcpListenSocket;
static PSocket_t *g_udpListenSocket;

_ptag static void serverConnectCallback(PSocket_t *sock, pbool_t success)
{
}

_ptag static void serverRecvCallback(PSocket_t *sock, const puint8_t *data, pint32_t len)
{
}

_ptag static void serverDisconnectCallback(PSocket_t *sock)
{
}

_ptag static void appListenCallback(PSocket_t *sock, PSocket_t *newSock)
{
}

_ptag static void discoverRecvCallback(PSocket_t *sock, const puint8_t *data, pint32_t len)
{
}

void CMConnectionPoll(void)
{

}

void CMStart(void)
{
    if (g_serverSocket == PNULL)
    {
        g_serverSocket = PSocketCreate(PSOCKET_TYPE_TCP);
        if (g_serverSocket)
        {
            g_serverSocket->connectCallback = serverConnectCallback;
            g_serverSocket->disconnectCallback = serverDisconnectCallback;
            g_serverSocket->recvCallback = serverRecvCallback;
        }
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
