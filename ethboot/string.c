#include <stddef.h>
#include <stdarg.h>
#include "defs.h"

/* Implementation taken from Linux kernel (linux/lib/string.c) */

int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	signed char res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

/* Taken from etherboot */
/*
ETHERBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: Dec/93
*/
/**************************************************************************
PRINTF and friends

	Formats:
		%X	- 4 byte ASCII (8 hex digits)
		%x	- 2 byte ASCII (4 hex digits)
		%b	- 1 byte ASCII (2 hex digits)
		%d	- decimal
		%c	- ASCII char
		%s	- ASCII string
		%I	- Internet address in x.x.x.x notation
	Note: width specification not supported
**************************************************************************/

static const char hex[] = "0123456789ABCDEF";
static char *vsprintf(char *buf, const char *fmt, va_list args)
{
	register char *p;
	char tmp[16];

	while (*fmt) {
		if (*fmt == '%') {	/* switch() uses more space */
			fmt++;
			if (*fmt == 'X') {
				register long h = va_arg(args, long);
				*(buf++) = hex[(h >> 28) & 0x0F];
				*(buf++) = hex[(h >> 24) & 0x0F];
				*(buf++) = hex[(h >> 20) & 0x0F];
				*(buf++) = hex[(h >> 16) & 0x0F];
				*(buf++) = hex[(h >> 12) & 0x0F];
				*(buf++) = hex[(h >> 8) & 0x0F];
				*(buf++) = hex[(h >> 4) & 0x0F];
				*(buf++) = hex[h & 0x0F];
			}
			if (*fmt == 'x') {
				register int h = va_arg(args, long);
				*(buf++) = hex[(h >> 12) & 0x0F];
				*(buf++) = hex[(h >> 8) & 0x0F];
				*(buf++) = hex[(h >> 4) & 0x0F];
				*(buf++) = hex[h & 0x0F];
			}
			if (*fmt == 'b') {
				register int h = va_arg(args, int);
				*(buf++) = hex[(h >> 4) & 0x0F];
				*(buf++) = hex[h & 0x0F];
			}
			if (*fmt == 'd') {
				register int dec = va_arg(args, int);
				p = tmp;
				if (dec < 0) {
					*(buf++) = '-';
					dec = -dec;
				}
				do {
					*(p++) = '0' + (dec % 10);
					dec = dec / 10;
				} while (dec);
				while ((--p) >= tmp)
					*(buf++) = *p;
			}
			if (*fmt == 'I') {
				union {
					long l;
					unsigned char c[4];
				} u;
				u.l = va_arg(args, long);
				buf = sprintf(buf, "%d.%d.%d.%d",
					      u.c[0], u.c[1], u.c[2], u.c[3]);
			}
			if (*fmt == 'c')
				*(buf++) = va_arg(args, int);
			if (*fmt == 's') {
				p = va_arg(args, char *);
				while (*p)
					*(buf++) = *p++;
			}
		} else
			*(buf++) = *fmt;
		fmt++;
	}
	*buf = '\0';
	return (buf);
}

char *sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	char *r;

	va_start(args, fmt);
	r = vsprintf(buf, fmt, args);
	va_end(args);
	return r;
}
