#ifndef PLATFORM_MC_H
#define PLATFORM_MC_H

#include "PlatformCTypes.h"
#include "PPrivate.h"

void PlatformTestCmdRecv(PrivateCtx_t *ctx, const char *msg, puint32_t len);

void PlatformMcOTAResultReport(PrivateCtx_t *ctx, pbool_t success);
void PlatformMcErrReport(PrivateCtx_t *ctx, const char *did, PErrorReport_t *err);
void PlatformMcStatusAlarm(PrivateCtx_t *ctx, const char *did, PStatusAlarm_t *alarm);
void PlatformMcTTS(PrivateCtx_t *ctx, const char *did, PTTSParameter_t *param);
void PlatformMcVoiceControl(PrivateCtx_t *ctx, const char *did, PVoiceCtrlParameter_t *param, const char *data, ptBool_t isLast, PVoiceCtrlExtension_t *ext, int extNum);
void PlatformMcEventInfoReport(PrivateCtx_t *ctx, const char *did, PEventInfo_t *event);
void PlatformMcSubDeviceUnbindReport(PrivateCtx_t *ctx, const char *did);
int PlatformMcSubDeviceOnOffline(PrivateCtx_t *ctx, const char *did, pbool_t online);
void PlatformMcDisconnectHandle(PrivateCtx_t *ctx);
void PlatformMcPostAll(PrivateCtx_t *ctx);
void PlatformMcRecv(PrivateCtx_t *ctx, const char *msg, puint32_t len);
void PlatformMcLogin(PrivateCtx_t *ctx);
void PlatformMcOnline(PrivateCtx_t *ctx);
void PlatformMcInitialize(void);
void PlatformMcPoll(PrivateCtx_t *ctx);

#endif
