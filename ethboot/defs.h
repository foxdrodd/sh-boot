#include "config.h"

extern void *memcpy (void *__dest, const void *__src, unsigned int __n);
extern int strlen (__const char *);
extern void *memset (void *, int, unsigned int);
extern int memcmp (const void *, const void *, unsigned int);

extern char *sprintf (char *, const char *, ...);
extern int printf (const char *, ...);
extern int getchar (void);

extern int putchar (int);

extern void sleep128 (int);
#define sleep(x) sleep128((x)*128)

#define MEMORY_ADDR_CONFIG (CONFIG_RAM_START+0x1000)
#define MEMORY_ADDR_KERNEL (CONFIG_RAM_BOOT+0x10000)

