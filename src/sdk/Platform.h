#ifndef PLATFORM_H
#define PLATFORM_H

typedef unsigned char ptBool_t;

typedef struct PPropertyInfo_st
{
    char *name;           //��������
    ptBool_t isText;      //�Ƿ�Ϊ�ı�����
    ptBool_t readonly;    //�Ƿ�Ϊֻ��
    unsigned short appid; //app��Ӧ������ID
    unsigned short sid;   //��·�豸��·����0 ��ʾΪ��·�豸��1��2��3 ��ʾ��·�豸�ĵ� 1��2��3 ·
}PPropertyInfo_t;

struct PrivateCtx_st;
typedef struct PlatformCtx_st
{
    struct PrivateCtx_st *private;
}PlatformCtx_t;

int PlatformResourceInfoSetNumValue(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, int value);
int PlatformResourceInfoSetTextValue(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, const char *value);
int PlatformResourceInfoRegister(PlatformCtx_t *ctx, const char *did, const char *rscName, unsigned short infoID, const char *infoName, ptBool_t isText);
int PlatformResourceRegister(PlatformCtx_t *ctx, const char *did, const char *name, unsigned short sid, unsigned short infoNum);

int PlatformPropertySetTextValue(PlatformCtx_t *ctx, unsigned short appid, const char *value);
int PlatformPropertySetNumValue(PlatformCtx_t *ctx, unsigned short appid, unsigned int value);
int PlatformPropertyRegister(PlatformCtx_t *ctx, PPropertyInfo_t *pInfo);
void PlatformSetModel(PlatformCtx_t *ctx, const char *model);
void PlatformSetDevid(PlatformCtx_t *ctx, const char *devid);
void PlatformSetPin(PlatformCtx_t *ctx, const char *pin);
void PlatformSetDeviceVersion(PlatformCtx_t *ctx, const char *version);
void PlatformStart(PlatformCtx_t *ctx);
PlatformCtx_t *PlatformCtxCreate(void);
void PlatformInitialize(void);
void PlatformPoll(PlatformCtx_t *ctx);

#endif // !PLATFORM_H
