#ifndef PUTIL_H
#define PUTIL_H

#include "PlatformCTypes.h"

puint16_t PUtilGetRandomNum(void);
char *PUtilAESEncodeBase64(const puint8_t *in, long len, const puint8_t *key, const puint8_t *iv);
puint8_t *PUtilBase64AESDecode(const char *in, const puint8_t *key, const puint8_t *iv, int *outlen);
#endif // !PUTIL_H
