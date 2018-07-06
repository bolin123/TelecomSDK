#include "HTTPRequest.h"
#include "MT2.h"

#define CONTENT_LENGTH_FLAG "Content-Length: "

static HTTPRequest_t g_httpRequestList;

_mtag void HTTPRequestInitialize(void)
{
    MT2ListInit(&g_httpRequestList);
}

_mtag static void handleEnd(HTTPRequest_t *request, char success)
{
    request->hasStart = 0;
    if(request->dataRecvCb)
    {
        request->dataRecvCb(request, MNULL, 0, success ? HTTP_REQ_ERROR_SUCCESS : HTTP_REQ_ERROR_FAIL);
    }
}

_mtag void HTTPRequestPoll(void)
{
    HTTPRequest_t *request;
    MT2ListForeach(&g_httpRequestList, request)
    {
        //超时
        if(request->hasStart && MT2TimeHasPast(request->validTime, request->timeout))
        {
            mlog("http request timeout :%s", request->url);
            handleEnd(request, 0);
        }
    }
}

_mtag static char parseRequestURL(HTTPRequest_t *request)
{
    char *p;

    const char *hostStart = request->url;
    p = strstr(hostStart, "/");
    if(!p)
    {
        p = request->url + strlen(request->url);
    }

    char host[1024] = {0};
    char port[10] = "80";
    memcpy(host, hostStart, (int)(p - hostStart));

    p = strchr(host, ':');
    if(p)
    {
        p[0] = 0;
        p++;
        strcpy(port, p);
    }

    request->host = malloc(strlen(host) + 1);
    strcpy(request->host, host);
    request->port = (unsigned short)atoi(port);
    mlog("request host %s:%s", request->host, port);

    //判断是不是域名
    int i;
    request->hostIsDomain = 0;
    for(i = 0; host[i] != 0; i++)
    {
        if((host[i] > '9' || host[i] < '0') && host[i] != '.')
        {
            request->hostIsDomain = true;
            break;
        }
    }

    return true;
}

_mtag HTTPRequest_t *HTTPRequestCreate(const char *url, const char *method)
{
    HTTPRequest_t *request = malloc(sizeof(HTTPRequest_t));

    request->method = method;
    request->url = malloc(strlen(url) + 1);
    request->headerBuf = malloc(512);

    if(strstr(url, "http://"))
    {
        strcpy(request->url, url + 7);
    }
    else
    {
        strcpy(request->url, url);
    }

    MT2ListInit(&request->params);
    MT2ListInit(&request->headers);
    request->host = MNULL;
    request->socket = MNULL;
    request->dataRecvCb = MNULL;
    request->data = MNULL;
    request->hasAddToList = 0;
    request->timeout = 10000;

    if(parseRequestURL(request))
    {

        MT2ListAdd(&g_httpRequestList, request);
        request->hasAddToList = 1;
        return request;
    }
    else
    {
        HTTPRequestDestroy(request);
        return MNULL;
    }
}

_mtag void HTTPRequestAddHeader(HTTPRequest_t *request, const char *key, const char *value)
{
    HTTPParam_t *param = malloc(sizeof(HTTPParam_t));
    param->key = malloc(strlen(key) + 1);
    param->value = malloc(strlen(value) + 1);
    strcpy(param->key, key);
    strcpy(param->value, value);
    MT2ListAdd(&request->headers, param);
}

_mtag static void destroyParam(HTTPParam_t *param)
{
    free(param->key);
    free(param->value);
    free(param);
}

_mtag void HTTPRequestDestroy(HTTPRequest_t *request)
{
    free(request->url);

    HTTPParam_t *param;

    if(request->hasAddToList)
    {
        MT2ListDel(request);
    }

    free(request->headerBuf);

    if(request->data)
    {
        free(request->data);
    }

    MT2ListForeach(&request->params, param)
    {
        destroyParam(param);
    }

    MT2ListForeach(&request->headers, param)
    {
        destroyParam(param);
    }

    if(request->host)
    {
        free(request->host);
        request->host = MNULL;
    }

    if(request->socket)
    {
        MT2SocketDestroy(request->socket);
    }

    
    free(request);
}

_mtag void HTTPRequestAddParam(HTTPRequest_t *request, const char *key, const char *value)
{
    HTTPParam_t *param = malloc(sizeof(HTTPParam_t));
    param->key = malloc(strlen(key) + 1);
    param->value = malloc(strlen(value) + 1);
    strcpy(param->key, key);
    strcpy(param->value, value);
    MT2ListAdd(&request->params, param);
}

_mtag static void connectCb(MT2Socket_t *sock, mbool_t result)
{
    HTTPRequest_t *request = sock->userdata;
    HTTPParam_t *param;

    mlog("connect %s result=%d", request->url, result);

    if(result == mfalse)
    {
        //handleEnd(request, 0);
        return;
    }

    request->rnCount = 0;
    request->headerBufCount = 0;
    request->respContentLength = 0;

    char reqData[2048] = {0};
    char *path = strstr(request->url, "/");
    if(path == MNULL)
    {
        path = "/";
    }
    sprintf(reqData, "%s %s", request->method, strstr(request->url, "/"));
    strcat(reqData, " HTTP/1.1\r\n");
    strcat(reqData, "Host: ");
    strcat(reqData, request->host);

    if(request->port != 80)
    {
        char tmp[10];
        strcat(reqData, ":");
        sprintf(tmp, "%d", request->port);
        strcat(reqData, tmp);
    }
    strcat(reqData, "\r\n");

    strcat(reqData, "Connection: Keep-Alive\r\n");
    strcat(reqData, "Content-Type: application/x-www-form-urlencoded\r\n");

    MT2ListForeach(&request->headers, param)
    {
        strcat(reqData, param->key);
        strcat(reqData, ": ");
        strcat(reqData, param->value);
        strcat(reqData, "\r\n");
    }

    unsigned short contentLen = 0;

    char hasParamBefore = 0;;

    //计算 Content-Length
    //带数据
    if(request->data)
    {
        contentLen = (unsigned short)strlen(request->data);
    }
    //带参数
    else
    {
        MT2ListForeach(&request->params, param)
        {
            if(hasParamBefore)
            {
                contentLen++;
            }

            contentLen = contentLen + (unsigned short)strlen(param->key) + (unsigned short)strlen(param->value) + 1;
            hasParamBefore = true;
        }
    }

    sprintf(reqData + strlen(reqData), "Content-Length: %d\r\n", contentLen);
    strcat(reqData, "\r\n");

    if(request->data)
    {
        strcat(reqData, request->data);
    }
    else
    {
        hasParamBefore = 0;
        MT2ListForeach(&request->params, param)
        {
            if(hasParamBefore)
            {
                strcat(reqData, "&");
            }

            strcat(reqData, param->key);
            strcat(reqData, "=");
            strcat(reqData, param->value);
            hasParamBefore = true;
        }
    }

    mlog("request");
    unsigned short i;
    unsigned short datalen = (unsigned short)strlen(reqData);
    for(i = 0; i < datalen; i++)
    {
        mprintf("%c", reqData[i]);
    }
    mprintf("\n");
    MT2SocketSend(request->socket, (unsigned char *)reqData, datalen);
}

_mtag static void disconnectCb(MT2Socket_t *sock)
{
    HTTPRequest_t *request = sock->userdata;

    mlog("disconnect %s", request->url);

    handleEnd(request, 0);
}


#define CHAR_DIGIT(ch) ((ch) - '0' < 10 ? (ch) - '0' : (ch) - 'a' + 10)

_mtag static long parseHexNumStr(const char *str)
{
    unsigned char len = (unsigned char)strlen(str);
    unsigned char i;
    long num = 0;
    char ch;
    for(i = 0; i < len; i++)
    {
        ch = str[i];
        num <<= 4;

        //转小写
        if(ch >= 'A' && ch <= 'Z')
        {
            ch += ('a' - 'A');
        }

        num |= CHAR_DIGIT(ch);
    }
    return num;
}

_mtag static void recvCb(MT2Socket_t *sock, const unsigned char *data, mint32_t len)
{
    HTTPRequest_t *request = sock->userdata;

    long i;

    request->validTime = MT2Time();

    //\r\n\r\n表示HTTP头结束
    //接收header中
    if(request->rnCount < 4)
    {
        for(i = 0; i < len; i++)
        {
            request->headerBuf[request->headerBufCount++] = data[i];
            request->headerBuf[request->headerBufCount] = 0;

            if(data[i] == '\r' || data[i] == '\n')
            {
                request->rnCount++;
            }
            else
            {
                request->rnCount = 0;
            }

            if(request->rnCount == 4)
            {
                mlog("recv header:%s", request->headerBuf);

                char *p = strstr((char *)request->headerBuf, CONTENT_LENGTH_FLAG);
                if(p)
                {
                    p += strlen(CONTENT_LENGTH_FLAG);
                    char lenStr[10] = {0};
                    char *p2 = strchr(p, '\r');
                    memcpy(lenStr, p, p2 - p);
                    request->respContentLength = atoi(lenStr);
                    request->respContentDataCount = 0;
                    mlog("respContentLength %d", request->respContentLength);
                }
                else
                {
                    request->chunkLen = 0;
                    request->chunkRNCount = 0;
                    request->headerBufCount = 0;
                }

                data = data + i + 1;
                len = (unsigned short)(len - i - 1);
                break;
            }
        }
    }

    if(request->rnCount < 4)
    {
        return;
    }

    //指定Content-Length
    if(request->respContentLength != 0)
    {
        mlog("recv data, len:%d count:%d contentLen:%d", len, request->respContentDataCount, request->respContentLength);
        request->respContentDataCount += len;

        if(request->dataRecvCb)
        {
            request->dataRecvCb(request, data, (unsigned short)len, HTTP_REQ_ERROR_NONE);
        }

        if(request->respContentDataCount >= request->respContentLength)
        {
            mlog("success");
            handleEnd(request, true);
        }
    }
    //transfer-encoding:chunked，chunk格式:[hex]\r\n[Data]\r\n[...]0\r\n
    else
    {
        for(i = 0; i < len; i++)
        {
            mprintf("%c", data[i]);

            //获取chunk长度
            if(request->chunkLen == 0)
            {
                if(data[i] == '\r' || data[i] == '\n')
                {
                    request->chunkRNCount++;
                }
                else
                {
                    request->chunkRNCount = 0;
                    request->headerBuf[request->headerBufCount++] = data[i];
                    request->headerBuf[request->headerBufCount] = '\0';
                }

                if(request->chunkRNCount == 2)
                {
                    request->chunkRNCount = 0;
                    request->headerBufCount = 0;
                    request->chunkLen = (unsigned short)parseHexNumStr(request->headerBuf);
                    mlog("chunk len:%d[%s]", request->chunkLen, request->headerBuf);
                    if(request->chunkLen == 0)
                    {
                        handleEnd(request, true);
                        return;
                    }
                    else
                    {
                        //chunk结尾的\r\n
                        request->chunkLen += 2;
                    }
                }
            }
            else
            {
                //屏蔽chunk结尾的\r\n
                if(request->chunkLen > 2)
                {
                    request->dataRecvCb(request, &data[i], 1, HTTP_REQ_ERROR_NONE);
                }
                request->chunkLen--;
            }
        }
    }
}

_mtag static void dnsResolveCb(const char *host, const char *ip, unsigned char success)
{
    mlog("dns %s->%s", host, ip);
    HTTPRequest_t *request;
    MT2ListForeach(&g_httpRequestList, request)
    {
        if(strcmp(request->host, host) == 0)
        {
            if(success)
            {
                request->socket = MT2SocketCreate(MT2SOCKET_TYPE_TCP);
                request->socket->connectCallback = connectCb;
                request->socket->disconnectCallback = disconnectCb;
                request->socket->recvCallback = recvCb;
                request->socket->userdata = request;
                mlog("connect");
                MT2SocketConnect(request->socket, ip, request->port);
            }
            else
            {
                //handleEnd(request, 0);
            }

            break;
        }
    }
}

_mtag void HTTPRequestSetData(HTTPRequest_t *request, const char *data)
{
    if(request->data)
    {
        free(request->data);
    }
    request->data = malloc(strlen(data) + 1);
    strcpy(request->data, data);
}

_mtag void HTTPRequestStart(HTTPRequest_t *request)
{
    request->hasStart = true;
    request->validTime = MT2Time();

    mlog("start, url=%s", request->url);
    if(request->hostIsDomain)
    {
        mlog("start dns resolve");
        MT2SocketDnsResolve(request->host, dnsResolveCb);
    }
    else
    {
        request->socket = MT2SocketCreate(MT2SOCKET_TYPE_TCP);
        request->socket->connectCallback = connectCb;
        request->socket->disconnectCallback = disconnectCb;
        request->socket->recvCallback = recvCb;
        request->socket->userdata = request;
        mlog("connect");
        MT2SocketConnect(request->socket, request->host, request->port);
    }
}


