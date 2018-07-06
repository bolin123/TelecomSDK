#ifndef AES_H
#define AES_H
#include "../PlatformCTypes.h"

int AesEncrypt(const unsigned char *in, unsigned char *out, int len,
               const unsigned char *key, int keylen);
int AesDecrypt(const unsigned char *in, unsigned char *out, int len,
               const unsigned char *key, int keylen);

long AES128CBCEncrypt(const unsigned char *in, unsigned char *out, long len, const unsigned char*key, const unsigned char *iv);
long AES128CBCDecrypt(const unsigned char *in, unsigned char *out, long len,
                      const unsigned char *key, const unsigned char *iv);

#endif // AES_H
