#ifndef PROPERTY_MANAGER_H
#define PROPERTY_MANAGER_H

#include "PPrivate.h"
//#include "Platform.h"

int PMModuleSetVersion(PrivateCtx_t *ctx, const char *did, const char *name, const char *version);
PMProperty_t *PMFindPropertyHeadByDid(PrivateCtx_t *ctx, const char *did);
ResourceInfo_t *PMFindResourceInfoByDid(PrivateCtx_t *ctx, const char *did);
PMProperty_t *PMFindPropertyByName(PrivateCtx_t *ctx, const char *did, const char *name);

/*资源管理*/
//int PMResourceInfoSetNumValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, int value);
//int PMResourceInfoSetTextValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, const char *value);
//int PMResourceInfoRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, pbool_t isText);
int PMResourceItemSet(PrivateCtx_t *ctx, const char *did, const char *rscName, unsigned short id, PCommonInfo_t *value, int valNum);
int PMResourceItemDel(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t id);
int PMResourceItemAdd(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t id, PCommonInfo_t *value, int valNum);
int PMResourceKeywordRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, const char *keyword, pbool_t isText);
int PMResourceRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, const char *idName, puint16_t sid);

/*设备管理*/
int PMPropertySetTextValue(PrivateCtx_t *ctx, const char *did, puint16_t id, const char *value);
int PMPropertySetNumValue(PrivateCtx_t *ctx, const char *did, puint16_t id, puint32_t value);
int PMPropertyRegister(PrivateCtx_t *ctx, const char *did, PPropertyInfo_t *pInfo);

/*从设备管理*/
//int PMSubPropertySetNumValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, puint32_t value);
//int PMSubPropertySetTextValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, const char *value);
//int PMSubPropertyRegister(PMSubDevice_t *subDevices, const char *did, PPropertyInfo_t *pInfo);
//int PMSubDeviceRegister(PrivateCtx_t *ctx, const char *did, const char *pin, const char *model);


void PMInitialize(void);
void PMPoll(void);

#endif

