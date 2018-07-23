#ifndef PLATFORM_MC_H
#define PLATFORM_MC_H

#include "PlatformCTypes.h"
#include "PPrivate.h"

void PlatformMcDisconnectHandle(PrivateCtx_t *ctx);
void PlatformMcPostAll(PrivateCtx_t *ctx);
void PlatformMcRecv(PrivateCtx_t *ctx, char *msg, pint16_t len);
void PlatformMcLogin(PrivateCtx_t *ctx);
void PlatformMcOnline(PrivateCtx_t *ctx);
void PlatformMcInitialize(void);
void PlatformMcPoll(PrivateCtx_t *ctx);

#endif
