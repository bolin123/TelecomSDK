#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "PlatformCTypes.h"
#include "Adapter/PSocket.h"
#include "Util/PList.h"

void HTTPRequestInitialize(void);
void HTTPRequestPoll(void);

#define HTTP_REQ_METHOD_POST "POST"
#define HTTP_REQ_METHOD_GET "GET"

typedef enum
{
    HTTP_REQ_ERROR_NONE = 0,
    HTTP_REQ_ERROR_FAIL,
    HTTP_REQ_ERROR_SUCCESS,
}HTTPRequestError_t;

typedef struct HTTPParam_st
{
    char *key;
    char *value;
    PLIST_ENTRY(struct HTTPParam_st);
}HTTPParam_t;

typedef struct HTTPRequest_st HTTPRequest_t;

typedef void(*HTTPRequestDataRecvCallback_t)(HTTPRequest_t *request, const unsigned char *data, unsigned short len, HTTPRequestError_t error);


struct HTTPRequest_st
{
    HTTPRequestDataRecvCallback_t dataRecvCb; //�ظ����ݻص�
    void *userData;                           //�û�����

    //������Ϣ
    PSocket_t *socket;
    const char *method;     //���󷽷�
    char *url;              //����URL
    char *host;             //����������
    char hostIsDomain : 1;  //��������������IP
    unsigned short port;          //����˿ں�
    HTTPParam_t params;     //�������
    HTTPParam_t headers;     //����ͷ
    char *data;             //��������

    unsigned short timeout; //��ʱʱ��ms

    char hasAddToList;
    char hasStart;
    ptime_t validTime;

    //
    char *headerBuf;
    unsigned short headerBufCount;
    unsigned char rnCount;

    //
    unsigned short chunkLen;
    unsigned char chunkRNCount;

    //
    long respContentDataCount;
    long respContentLength;

    PLIST_ENTRY(struct HTTPRequest_st);
};


HTTPRequest_t *HTTPRequestCreate(const char *url, const char *method);

void HTTPRequestDestroy(HTTPRequest_t *request);

void HTTPRequestSetData(HTTPRequest_t *request, const char *data);
void HTTPRequestAddParam(HTTPRequest_t *request, const char *key, const char *value);
void HTTPRequestAddHeader(HTTPRequest_t *request, const char *key, const char *value);
void HTTPRequestStart(HTTPRequest_t *request);

#endif // HTTP_REQUEST_H



