#include "HTTPRequest.h"
#include "PPrivate.h"

#define CONTENT_LENGTH_FLAG "Content-Length: "

static HTTPRequest_t g_httpRequestList;

_ptag void HTTPRequestInitialize(void)
{
    PListInit(&g_httpRequestList);
}

_ptag static void handleEnd(HTTPRequest_t *request, char success)
{
    request->hasStart = 0;
    if(request->dataRecvCb)
    {
        request->dataRecvCb(request, PNULL, 0, success ? HTTP_REQ_ERROR_SUCCESS : HTTP_REQ_ERROR_FAIL);
    }
}

_ptag void HTTPRequestPoll(void)
{
    HTTPRequest_t *request;
    PListForeach(&g_httpRequestList, request)
    {
        //超时
        if(request->hasStart && PTimeHasPast(request->validTime, request->timeout))
        {
            plog("http request timeout :%s", request->url);
            handleEnd(request, 0);
        }
    }
}

_ptag static char parseRequestURL(HTTPRequest_t *request)
{
    char *p;

    const char *hostStart = request->url;
    p = strstr(hostStart, "/");
    if(!p)
    {
        p = request->url + strlen(request->url);
    }

    char host[1024] = { 0 };
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
    plog("request host %s:%s", request->host, port);

    //判断是不是域名
    int i;
    request->hostIsDomain = 0;
    for(i = 0; host[i] != 0; i++)
    {
        if((host[i] > '9' || host[i] < '0') && host[i] != '.')
        {
            request->hostIsDomain = ptrue;
            break;
        }
    }

    return ptrue;
}

_ptag HTTPRequest_t *HTTPRequestCreate(const char *url, const char *method)
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

    PListInit(&request->params);
    PListInit(&request->headers);
    request->host = PNULL;
    request->socket = PNULL;
    request->dataRecvCb = PNULL;
    request->data = PNULL;
    request->hasAddToList = 0;
    request->timeout = 10000;

    if(parseRequestURL(request))
    {

        PListAdd(&g_httpRequestList, request);
        request->hasAddToList = 1;
        return request;
    }
    else
    {
        HTTPRequestDestroy(request);
        return PNULL;
    }
}

_ptag void HTTPRequestAddHeader(HTTPRequest_t *request, const char *key, const char *value)
{
    HTTPParam_t *param = malloc(sizeof(HTTPParam_t));
    param->key = malloc(strlen(key) + 1);
    param->value = malloc(strlen(value) + 1);
    strcpy(param->key, key);
    strcpy(param->value, value);
    PListAdd(&request->headers, param);
}

_ptag static void destroyParam(HTTPParam_t *param)
{
    free(param->key);
    free(param->value);
    free(param);
}

_ptag void HTTPRequestDestroy(HTTPRequest_t *request)
{
    free(request->url);

    HTTPParam_t *param;

    if(request->hasAddToList)
    {
        PListDel(request);
    }

    free(request->headerBuf);

    if(request->data)
    {
        free(request->data);
    }

    PListForeach(&request->params, param)
    {
        destroyParam(param);
    }

    PListForeach(&request->headers, param)
    {
        destroyParam(param);
    }

    if(request->host)
    {
        free(request->host);
        request->host = PNULL;
    }

    if(request->socket)
    {
        PSocketDestroy(request->socket);
    }


    free(request);
}

_ptag void HTTPRequestAddParam(HTTPRequest_t *request, const char *key, const char *value)
{
    HTTPParam_t *param = malloc(sizeof(HTTPParam_t));
    param->key = malloc(strlen(key) + 1);
    param->value = malloc(strlen(value) + 1);
    strcpy(param->key, key);
    strcpy(param->value, value);
    PListAdd(&request->params, param);
}

_ptag static void connectCb(PSocket_t *sock, pbool_t result)
{
    HTTPRequest_t *request = sock->userdata;
    HTTPParam_t *param;
    pbool_t extHead = pfalse;

    plog("connect %s result=%d", request->url, result);

    if(result == pfalse)
    {
        //handleEnd(request, 0);
        return;
    }

    request->rnCount = 0;
    request->headerBufCount = 0;
    request->respContentLength = 0;

    char reqData[2048] = { 0 };
    char *path = strstr(request->url, "/");
    if(path == PNULL)
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

    PListForeach(&request->headers, param)
    {
        strcat(reqData, param->key);
        strcat(reqData, ": ");
        strcat(reqData, param->value);
        strcat(reqData, "\r\n");
        extHead = ptrue;
    }
    if(!extHead)
    {
        strcat(reqData, "Content-Type: application/x-www-form-urlencoded\r\n");
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
        PListForeach(&request->params, param)
        {
            if(hasParamBefore)
            {
                contentLen++;
            }

            contentLen = contentLen + (unsigned short)strlen(param->key) + (unsigned short)strlen(param->value) + 1;
            hasParamBefore = ptrue;
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
        PListForeach(&request->params, param)
        {
            if(hasParamBefore)
            {
                strcat(reqData, "&");
            }

            strcat(reqData, param->key);
            strcat(reqData, "=");
            strcat(reqData, param->value);
            hasParamBefore = ptrue;
        }
    }

    plog("request");
    unsigned short i;
    unsigned short datalen = (unsigned short)strlen(reqData);
    for(i = 0; i < datalen; i++)
    {
        pprintf("%c", reqData[i]);
    }
    pprintf("\n");
    PSocketSend(request->socket, (unsigned char *)reqData, datalen);
}

_ptag static void disconnectCb(PSocket_t *sock)
{
    HTTPRequest_t *request = sock->userdata;

    plog("disconnect %s", request->url);

    handleEnd(request, 0);
}


#define CHAR_DIGIT(ch) ((ch) - '0' < 10 ? (ch) - '0' : (ch) - 'a' + 10)

_ptag static long parseHexNumStr(const char *str)
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

_ptag static void recvCb(PSocket_t *sock, const unsigned char *data, unsigned int len)
{
    HTTPRequest_t *request = sock->userdata;

    unsigned int i;

    request->validTime = PlatformTime();

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
                plog("recv header:%s", request->headerBuf);

                char *p = strstr((char *)request->headerBuf, CONTENT_LENGTH_FLAG);
                if(p)
                {
                    p += strlen(CONTENT_LENGTH_FLAG);
                    char lenStr[10] = { 0 };
                    char *p2 = strchr(p, '\r');
                    memcpy(lenStr, p, p2 - p);
                    request->respContentLength = atoi(lenStr);
                    request->respContentDataCount = 0;
                    plog("respContentLength %d", request->respContentLength);
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
        plog("recv data, len:%d count:%d contentLen:%d", len, request->respContentDataCount, request->respContentLength);
        request->respContentDataCount += len;

        if(request->dataRecvCb)
        {
            request->dataRecvCb(request, data, (unsigned short)len, HTTP_REQ_ERROR_NONE);
        }

        if(request->respContentDataCount >= request->respContentLength)
        {
            plog("success");
            handleEnd(request, ptrue);
        }
    }
    //transfer-encoding:chunked，chunk格式:[hex]\r\n[Data]\r\n[...]0\r\n
    else
    {
        for(i = 0; i < len; i++)
        {
            pprintf("%c", data[i]);

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
                    plog("chunk len:%d[%s]", request->chunkLen, request->headerBuf);
                    if(request->chunkLen == 0)
                    {
                        handleEnd(request, ptrue);
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

_ptag static void dnsResolveCb(const char *host, const char *ip, unsigned char success)
{
    plog("dns %s->%s", host, ip);
    HTTPRequest_t *request;
    PListForeach(&g_httpRequestList, request)
    {
        if(strcmp(request->host, host) == 0)
        {
            if(success)
            {
                request->socket = PSocketCreate(PSOCKET_TYPE_TCP);
                request->socket->connectCallback = connectCb;
                request->socket->disconnectCallback = disconnectCb;
                request->socket->recvCallback = recvCb;
                request->socket->userdata = request;
                plog("connect");
                PSocketConnect(request->socket, ip, request->port);
            }
            else
            {
                //handleEnd(request, 0);
            }

            break;
        }
    }
}

_ptag void HTTPRequestSetData(HTTPRequest_t *request, const char *data)
{
    if(request->data)
    {
        free(request->data);
    }
    request->data = malloc(strlen(data) + 1);
    strcpy(request->data, data);
}

_ptag void HTTPRequestStart(HTTPRequest_t *request)
{
    request->hasStart = ptrue;
    request->validTime = PlatformTime();

    plog("start, url=%s", request->url);
    if(request->hostIsDomain)
    {
        plog("start dns resolve");
        PSocketDnsResolve(request->host, dnsResolveCb);
    }
    else
    {
        request->socket = PSocketCreate(PSOCKET_TYPE_TCP);
        request->socket->connectCallback = connectCb;
        request->socket->disconnectCallback = disconnectCb;
        request->socket->recvCallback = recvCb;
        request->socket->userdata = request;
        plog("connect");
        PSocketConnect(request->socket, request->host, request->port);
    }
}


