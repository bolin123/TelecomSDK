#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#define ROM_FUNC 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef unsigned long long SysTime_t;
typedef unsigned char       vtuint8_t;
typedef signed char         vtint8_t;
typedef unsigned short      vtuint16_t;
typedef signed short        vtint16_t;
typedef int   vtint32_t;
typedef unsigned int  vtuint32_t;
typedef signed long long    vtint64_t;
typedef unsigned long long  vtuint64_t;
typedef unsigned char       vtbool;
typedef vtint32_t vtsize_t;

#undef uint8_t
#define uint8_t vtuint8_t
#undef int8_t
#define int8_t vtint8_t
#undef uint16_t
#define uint16_t vtuint16_t
#undef int16_t
#define int16_t vtint16_t
#undef uint32_t
#define uint32_t vtuint32_t
#undef int32_t
#define int32_t vtint32_t
#undef uint64_t
#define uint64_t vtuint64_t
#undef int64_t
#define int64_t vtint64_t
#undef size_t
#define size_t vtsize_t
#undef bool
#define bool vtbool

#undef true
#define true (1)
#undef false
#define false (0)

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef BIT
#define BIT(nr)                 (1UL << (nr))
#endif

#ifndef KB
#define KB(n) ((n) * 1024UL)
#endif

#endif // SYS_TYPES_H
