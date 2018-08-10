#ifndef Platform_OTA_H
#define Platform_OTA_H

#include "PPrivate.h"

void PlatformOTAStartDownload(PrivateCtx_t *ctx, const char *did, PUpgradeInfo_t *upgradeInfo);
void PlatformOTAStopDownlaod(PrivateCtx_t *ctx);

#endif // !Platform_OTA_H
