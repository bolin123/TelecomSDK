#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "c_types.h"
#include "osapi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#undef ROM_FUNC
#define ROM_FUNC ICACHE_FLASH_ATTR

//Ìæ»»±ê×¼¿âº¯Êý
void *pvPortMalloc( size_t xWantedSize ) ;
void vPortFree( void *pv );
void *pvPortRealloc(void * p, size_t size);
void os_printf_plus(const char *, ...);

void ets_memcpy(void *, const void *, size_t);
void ets_memset(void *, int, size_t);
int ets_memcmp(const void *, const void *, size_t);

char *ets_strstr(const char *, const char *);
char *ets_strchr(const char *, char ch);
void ets_sprintf(char *, const char *, ...);
void ets_strcpy(char *, const char *);
int ets_strcmp(const char *, const char *);
int ets_strlen(const char *);
void ets_strncpy(char *, const char *, size_t);
void *dbg_malloc(size_t size);
void *dbg_free(void *p);

#define printf os_printf_plus

#if 0
#define malloc(size) dbg_malloc(size);
#define free(p) dbg_free(p);
#else
#define malloc pvPortMalloc
#define free(p) vPortFree(p);
#endif
#define realloc pvPortRealloc
#define memcmp ets_memcmp
#undef memcpy
#define memcpy ets_memcpy
#undef memmove
#define memmove ets_memmove
#undef memset
#define memset ets_memset
#define strcmp ets_strcmp
#undef strcpy
#define strcpy ets_strcpy
#define strlen ets_strlen
#define strncmp ets_strncmp
#undef strncpy
#define strncpy ets_strncpy
#undef sprintf
#define sprintf ets_sprintf
#undef strstr
#define strstr ets_strstr
#undef strchr
#define strchr ets_strchr
#undef strcat
#define strcat ets_strcat

#ifdef __WIN__
#define _FILE (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#undef __packed
#define __packed 
#endif

#define WIFI

#endif // HAL_CONFIG_H


