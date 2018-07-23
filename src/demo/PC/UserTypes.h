#ifndef USER_TYPES_H
#define USER_TYPES_H

#define ROM_FUNC

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef unsigned long long ustime_t;
typedef unsigned char       usuint8_t;
typedef signed char         usint8_t;
typedef unsigned short      usuint16_t;
typedef signed short        usint16_t;
typedef int   usint32_t;
typedef unsigned int  usuint32_t;
typedef signed long long    usint64_t;
typedef unsigned long long  usuint64_t;
typedef unsigned char       usbool;
typedef uint32_t ussize_t;

#undef uint8_t
#define uint8_t usuint8_t
#undef int8_t
#define int8_t usint8_t
#undef uint16_t
#define uint16_t usuint16_t
#undef int16_t
#define int16_t usint16_t
#undef uint32_t
#define uint32_t usuint32_t
#undef int32_t
#define int32_t usint32_t
#undef uint64_t
#define uint64_t usuint64_t
#undef int64_t
#define int64_t usint64_t
#undef size_t
#define size_t ussize_t
#undef bool
#define bool usbool

#undef true
#define true (1)
#undef false
#define false (0)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define uprintf printf
#define ulog(...) uprintf("[PF] %s[%d]", __FUNCTION__, __LINE__);uprintf(__VA_ARGS__);uprintf("\n");
#endif // SYS_TYPES_H
