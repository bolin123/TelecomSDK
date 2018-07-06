#ifndef PSOCKET_ADAPTER_H
#define PSOCKET_ADAPTER_H

//Socket����
typedef enum
{
    PSOCKET_TYPE_UDP,
    PSOCKET_TYPE_TCP,
}PSocketType_t;

struct PSocket_st;
typedef struct PSocket_st PSocket_t;

typedef void (*PSocketRecvCallback_t)(PSocket_t *sock, const unsigned char *data, unsigned int len);
typedef void (*PSocketListenCallback_t)(PSocket_t *sock, PSocket_t *newSock);
typedef void (*PSocketConnectCallback_t)(PSocket_t *sock, unsigned char success);
typedef void (*PSocketDisconnectCallback_t)(PSocket_t *sock);

struct PSocket_st
{
    PSocketType_t type;
    unsigned char connected;
    PSocketRecvCallback_t recvCallback;
    PSocketConnectCallback_t connectCallback;
    PSocketDisconnectCallback_t disconnectCallback;
    PSocketListenCallback_t listenCallback;
    void *userdata;
    void *pri;
};

//����socket
PSocket_t *PSocketCreate(PSocketType_t sockType);

//����socket
void PSocketDestroy(PSocket_t *sock);

//����
void PSocketConnect(PSocket_t *sock, const char *ip, unsigned short port);

//��ʼ����
void PSocketStartListen(PSocket_t *sock, unsigned short port);

//TCP����
unsigned int PSocketSend(PSocket_t *sock, const unsigned char *data, unsigned int len);

//��ȡUDPԶ�˵�ַ
void PSocketGetRemoteAddr(PSocket_t *sock, char *ip, unsigned short *port);

//UDP����
unsigned int PSocketSendTo(PSocket_t *sock, const char *ip, unsigned short port, const unsigned char *data, unsigned int len);

//�Ͽ�����
void PSocketDisconnect(PSocket_t *sock);

typedef void (*PSocketDnsResolveCallback_t)(const char *host, const char *ip, unsigned char success);

//DNS����
void PSocketDnsResolve(const char *host, PSocketDnsResolveCallback_t callback);

#endif // PSOCKET_ADAPTER_H
