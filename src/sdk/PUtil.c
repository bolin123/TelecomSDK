#include "PUtil.h"
#include "Util/Aes.h"
#include "Util/Base64.h"

//xorshiftËæ»úÊýËã·¨
static puint32_t x = 123456789UL, y = 567819012UL, z = 321456780UL, w = 1234UL;
_ptag static puint32_t xorshift128(void)
{
    puint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

_ptag static puint32_t random(void)
{
    return xorshift128();
}

_ptag static void randomSeed(puint32_t seed)
{
    x = seed;
    y = random();
    z = random();
    w = random();
}

_ptag puint16_t PUtilGetRandomNum(void)
{
    randomSeed(PlatformTime());
    return (puint16_t)(random() & 0xffff);
}

_ptag char *PUtilAESEncodeBase64(const puint8_t *in, long len, const puint8_t *key, const puint8_t *iv)
{
    int outlen, baselen;
    char *base64;
    char *out = (char *)malloc(len + 16);

    if (out)
    {
        outlen = AES128CBCEncrypt(in, out, len, key, iv);
        base64 = Base64Encode(out, outlen, &baselen);
        free(out);
        return base64;
    }
    return PNULL;
}

_ptag puint8_t *PUtilBase64AESDecode(const char *in, const puint8_t *key, const puint8_t *iv, int *outlen)
{
    int blen;
    puint8_t *bdata = PNULL;

    bdata = Base64Decode(in, strlen(in), &blen);
    if (bdata)
    {
        *outlen = AES128CBCDecrypt(bdata, bdata, blen, key, iv);
        return bdata;
    }
    *outlen = 0;
    return PNULL;
}

