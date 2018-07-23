#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "PlatformCTypes.h"
#include "PPrivate.h"

/*
typedef enum
{
    CM_SERVER_STATUS_IDLE = 0,
    CM_SERVER_STATUS_LS_CONNECTING,
    CM_SERVER_STATUS_LS_CONNECTED,
    CM_SERVER_STATUS_CS_CONNECTING,
    CM_SERVER_STATUS_CS_CONNECTED,
    CM_SERVER_STATUS_ONLINE,
}CMServerStatus_t;
*/

void CMServerSendData(PrivateCtx_t *ctx, const puint8_t *data, puint32_t len);
pbool_t CMIsOnlineOrClientConnect(PrivateCtx_t *ctx);
CMServerStatus_t CMServerStatus(PrivateCtx_t *ctx);
void CMServerLoginSuccess(PrivateCtx_t *ctx, const char *ip, puint16_t port);
void CMServerOnline(PrivateCtx_t *ctx);

void CMStart(PrivateCtx_t *ctx);
void CMInitialize(void);
void CMPoll(PrivateCtx_t *ctx);

#endif // !CONNECTION_MANAGER_H
