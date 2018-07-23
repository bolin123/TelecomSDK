#ifndef PROPERTY_MANAGER_H
#define PROPERTY_MANAGER_H

#include "PPrivate.h"
#include "Platform.h"



/*资源管理*/
int PMResourceInfoSetNumValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, int value);
int PMResourceInfoSetTextValue(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, const char *value);
int PMResourceInfoRegister(PrivateCtx_t *ctx, const char *did, const char *rscName, puint16_t infoID, const char *infoName, pbool_t isText);
int PMResourceRegister(PrivateCtx_t *ctx, const char *did, const char *name, puint16_t sid, puint16_t infoNum);

/*设备管理*/
int PMPropertySetTextValue(PMProperty_t *properties, puint16_t id, const char *value);
int PMPropertySetNumValue(PMProperty_t *properties, puint16_t id, puint32_t value);
int PMPropertyRegister(PMProperty_t *properties, PPropertyInfo_t *pInfo);
//int PMDeviceRegister(const char *did);
//PMProperty_t *PMPropertyHead(void);

/*从设备管理*/
int PMSubPropertySetNumValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, puint32_t value);
int PMSubPropertySetTextValue(PMSubDevice_t *subDevices, const char *did, puint16_t id, const char *value);
int PMSubPropertyRegister(PMSubDevice_t *subDevices, const char *did, PPropertyInfo_t *pInfo);
int PMSubDeviceRegister(PMSubDevice_t *subDevices, const char *did, const char *pin);
//PMSubDevice_t *PMSubDeivceHead(void);


void PMInitialize(void);
void PMPoll(void);

#endif

