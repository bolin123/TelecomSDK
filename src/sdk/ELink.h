#ifndef ELINK_H
#define ELINK_H

#include "PPrivate.h"

void ELinkLinkdown(void);
void ELinkUpgradeResultReport(PrivateCtx_t *ctx, pbool_t success);
void ELinkSubdeviceOnoffLine(PrivateCtx_t *ctx, const char *did, pbool_t online);
void ELinkStart(void);
void ELinkStop(void);
void ELinkInitialize(void);
void ELinkPoll(PrivateCtx_t *ctx);

#endif

