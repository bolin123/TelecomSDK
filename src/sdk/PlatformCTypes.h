#ifndef PLATFORM_CTYPES_H
#define PLATFORM_CTYPES_H

#include "PlatformOption.h"

#ifndef _ptag
#define _ptag
#endif

//¿‡–Õ
typedef unsigned char       puint8_t;
typedef signed char         pint8_t;
typedef unsigned short      puint16_t;
typedef signed short        pint16_t;
typedef unsigned long       puint32_t;
typedef signed long         pint32_t;
typedef signed long long    pint64_t;
typedef unsigned long long  puint64_t;
typedef unsigned char       pbool_t;
typedef puint32_t           ptime_t;

#define ptrue (1)
#define pfalse (0)

#define PNULL ((void *)0)

#ifndef __ppacked
#define __ppacked __attribute__((packed))
#endif

#ifndef PFILE
#define PFILE __FILE__
#endif

#ifndef PFUNC
#define PFUNC __func__
#endif

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef PLINE
#define PLINE __LINE__
#endif

#ifdef pprintf
#ifndef plog
#define plog(...) pprintf("[PF] %s[%d]:", PFUNC, PLINE);pprintf(__VA_ARGS__);pprintf("\n");
#endif
#else
#define pprintf
#define plog(...)
#endif

#define ASSERT(exp) if(!(exp))plog("ERROR: ASSERT FAIL!!!, %s [%s]", #exp, PFUNC);

#endif
