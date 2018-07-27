#include "ConnectionManager.h"
#include "Adapter/PAdapter.h"
#include "Adapter/PSocket.h"
#include "PlatformMc.h"

static PSocket_t *g_serverSocket = PNULL;
static PSocket_t *g_tcpListenSocket;
static PSocket_t *g_udpListenSocket;
//static CMServerStatus_t g_serverStatus = CM_SERVER_STATUS_IDLE;
static ptime_t g_lastServerStatusTime = 0;

_ptag void CMServerSendData(PrivateCtx_t *ctx, const puint8_t *data, puint32_t len)
{
    plog("%s", (char *)data);
    PSocketSend(ctx->serverSocket, data, len);
}

_ptag CMServerStatus_t CMServerStatus(PrivateCtx_t *ctx)
{
    return ctx->serverStatus;
}

_ptag pbool_t CMServerConnected(PrivateCtx_t *ctx)
{
    return (ctx->serverStatus == CM_SERVER_STATUS_ONLINE);
}

_ptag pbool_t CMIsOnlineOrClientConnect(PrivateCtx_t *ctx)
{
    if(ctx->serverStatus == CM_SERVER_STATUS_ONLINE)
    {
        return ptrue;
    }
    return pfalse;
}

_ptag static void updateServerStatus(PrivateCtx_t *ctx, CMServerStatus_t status)
{
    plog("status = %d", status);
    if(ctx->serverStatus != status)
    {
        if(ctx->serverStatus == CM_SERVER_STATUS_IDLE)
        {
            PPrivateEventEmit(ctx, PEVENT_SERVER_ON_OFFLINE, (void *)pfalse);
        }
        else if(ctx->serverStatus == CM_SERVER_STATUS_ONLINE)
        {
            PPrivateEventEmit(ctx, PEVENT_SERVER_ON_OFFLINE, (void *)ptrue);
        }
        ctx->serverStatus = status;
    }
    ctx->lastServerStatusTime = PlatformTime();
}

_ptag static void serverConnectCallback(PSocket_t *sock, pbool_t success)
{
    PrivateCtx_t *ctx = (PrivateCtx_t *)sock->userdata;

    plog("socket = %p, success = %d", sock, success);

    if(success)
    {
        if(ctx->serverStatus == CM_SERVER_STATUS_LS_CONNECTING)
        {
            updateServerStatus(ctx, CM_SERVER_STATUS_LS_CONNECTED);
            PlatformMcLogin(ctx);
        }
        else if(ctx->serverStatus = CM_SERVER_STATUS_CS_CONNECTING)
        {
            updateServerStatus(ctx, CM_SERVER_STATUS_CS_CONNECTED);
            PlatformMcOnline(ctx);
        }
    }
    else
    {
        PlatformMcDisconnectHandle(ctx);
    }
}

_ptag static void serverRecvCallback(PSocket_t *sock, const puint8_t *data, puint32_t len)
{
    PrivateCtx_t *ctx = (PrivateCtx_t *)sock->userdata;

    plog("socket = %p", sock);
    if(sock == ctx->serverSocket)
    {
        PlatformMcRecv(ctx, data, len);
    }
}

_ptag static void serverDisconnectCallback(PSocket_t *sock)
{
    PrivateCtx_t *ctx = (PrivateCtx_t *)sock->userdata;

    plog("socket = %p", sock);
    updateServerStatus(ctx, CM_SERVER_STATUS_IDLE);
    PlatformMcDisconnectHandle(ctx);
}

_ptag static void appListenCallback(PSocket_t *sock, PSocket_t *newSock)
{
}

_ptag static void discoverRecvCallback(PSocket_t *sock, const puint8_t *data, pint32_t len)
{
}

_ptag static PSocket_t *initSocket(PSocketType_t type, PrivateCtx_t *ctx)
{
    PSocket_t *socket;

    socket = PSocketCreate(type);

    plog("socket = %p", socket);
    if(socket)
    {
        socket->connectCallback = serverConnectCallback;
        socket->disconnectCallback = serverDisconnectCallback;
        socket->recvCallback = serverRecvCallback;
        socket->userdata = (void *)ctx;
        return socket;
    }
    return PNULL;
}

static PrivateCtx_t *g_ctxTmp;
_ptag static void dnsResolve(const char *host, const char *ip, unsigned char success)
{
    plog("host:%s, ip:%s, success = %d", host, ip, success);
    if(success)
    {
        PSocketDestroy(g_ctxTmp->serverSocket);
        g_ctxTmp->serverSocket = initSocket(PSOCKET_TYPE_TCP, g_ctxTmp);
        PSocketConnect(g_ctxTmp->serverSocket, ip, PLATFORM_SERVER_PORT);
    }
}

_ptag static void serverManagerPoll(PrivateCtx_t *ctx)
{
    if(PIsLinkup())
    {
        if(CM_SERVER_STATUS_IDLE == ctx->serverStatus)
        {
            if(ctx->lastServerStatusTime == 0
                || PTimeHasPast(ctx->lastServerStatusTime, 30000))
            {
                plog("dns start...");
                g_ctxTmp = ctx;
                PSocketDnsResolve(PLATFORM_SERVER_URL, dnsResolve);
                updateServerStatus(ctx, CM_SERVER_STATUS_LS_CONNECTING);
            }
        }
        else if (CM_SERVER_STATUS_ONLINE == ctx->serverStatus)
        {
        }
        else
        {
            if(PTimeHasPast(ctx->lastServerStatusTime, 20000))
            {
                // TODO:timeout
                updateServerStatus(ctx, CM_SERVER_STATUS_IDLE);
            }
        }

    }
}

_ptag void CMServerLoginSuccess(PrivateCtx_t *ctx, const char *ip, puint16_t port)
{
    plog("%s: %d", ip, port);
    updateServerStatus(ctx, CM_SERVER_STATUS_CS_CONNECTING);
    if(ctx->serverSocket->connected)
    {
        PSocketDisconnect(ctx->serverSocket);
    }
    PSocketConnect(ctx->serverSocket, ip, port);
}

_ptag void CMServerReconnect(PrivateCtx_t *ctx)
{
    if(ctx->serverSocket->connected)
    {
        PSocketDisconnect(ctx->serverSocket);
    }
    updateServerStatus(ctx, CM_SERVER_STATUS_IDLE);
}

_ptag void CMServerOnline(PrivateCtx_t *ctx)
{
    plog("");
    updateServerStatus(ctx, CM_SERVER_STATUS_ONLINE);
    PlatformMcPostAll(ctx);
}

_ptag void CMStart(PrivateCtx_t *ctx)
{
    if(ctx->serverSocket == PNULL)
    {
        ctx->serverSocket = initSocket(PSOCKET_TYPE_TCP, ctx);
    }

    //������TCP����
    if(g_tcpListenSocket == PNULL)
    {
        g_tcpListenSocket = PSocketCreate(PSOCKET_TYPE_TCP);
        g_tcpListenSocket->listenCallback = appListenCallback;
        g_tcpListenSocket->userdata = (void *)ctx;
        PSocketStartListen(g_tcpListenSocket, 7681);
    }

    //������UDP����
    if(g_udpListenSocket == PNULL)
    {
        g_udpListenSocket = PSocketCreate(PSOCKET_TYPE_UDP);
        g_udpListenSocket->recvCallback = discoverRecvCallback;
        g_udpListenSocket->userdata = (void *)ctx;
        PSocketStartListen(g_udpListenSocket, 7680);
    }

}

_ptag void CMInitialize(void)
{
}

_ptag void CMPoll(PrivateCtx_t *ctx)
{
    serverManagerPoll(ctx);
    //todo:if linkup change, clear slave socket
}
