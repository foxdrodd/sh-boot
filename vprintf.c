/*
 *  Most part is taken from linux/lib/vsprintf.c
 */
/*
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdarg.h>
#include "string.h"
#include "ctype.h"

typedef long size_t;
int printf(const char *, ...);

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

int putchar(char ch)
{
	extern void putDebugChar(char ch);

	if (ch == '\n')
		putDebugChar('\r');
	putDebugChar(ch);
	return (unsigned char)ch;
}

int puts(const char *s)
{
	while (*s)
		putchar(*s++);
	return 0;
}

static int number(long long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[16];
	const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	int i, n = 0;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = digits[do_div(num, base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0) {
			putchar(' ');
			n++;
		}
	if (sign) {
		putchar(sign);
		n++;
	}
	if (type & SPECIAL) {
		if (base == 8) {
			putchar('0');
			n++;
		} else if (base == 16) {
			putchar('0');
			putchar(digits[33]);
			n += 2;
		}
	}
	if (!(type & LEFT))
		while (size-- > 0) {
			putchar(c);
			n++;
		}
	while (i < precision--) {
		putchar('0');
		n++;
	}
	while (i-- > 0) {
		putchar(tmp[i]);
		n++;
	}
	while (size-- > 0) {
		putchar(' ');
		n++;
	}
	return n;
}

/**
 * vprintf - Format a string and output to debug console.
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want sprintf instead.
 */
int vprintf(const char *fmt, va_list args)
{
	int len, n;
	unsigned long long num;
	int i, base;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
	/* 'z' support added 23/7/1999 S.H.    */
	/* 'z' changed to 'Z' --davidm 1/25/99 */

	for (n = 0; *fmt; ++fmt) {
		if (*fmt != '%') {
			putchar(*fmt);
			n++;
			continue;
		}

		/* process flags */
		flags = 0;
	      repeat:
		++fmt;		/* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0) {
					putchar(' ');
					n++;
				}
			putchar((unsigned char)va_arg(args, int));
			n++;
			while (--field_width > 0) {
				putchar(' ');
				n++;
			}
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--) {
					putchar(' ');
					n++;
				}
			for (i = 0; i < len; ++i) {
				putchar(*s++);
				n++;
			}
			while (len < field_width--) {
				putchar(' ');
				n++;
			}
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			n += number((unsigned long)va_arg(args, void *), 16,
				    field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = n;
			} else if (qualifier == 'Z') {
				size_t *ip = va_arg(args, size_t *);
				*ip = n;
			} else {
				int *ip = va_arg(args, int *);
				*ip = n;
			}
			continue;

		case '%':
			putchar('%');
			n++;
			continue;

		case 'I':
			{
				union {
					long l;
					unsigned char c[4];
				} u;
				u.l = va_arg(args, long);
				printf("%d.%d.%d.%d", u.c[0], u.c[1], u.c[2],
				       u.c[3]);
			}
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			putchar('%');
			n++;
			if (*fmt) {
				putchar(*fmt);
				n++;
			} else
				--fmt;
			continue;
		}
		if (qualifier == 'L')
			num = va_arg(args, long long);
		else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long)num;
		} else if (qualifier == 'Z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (signed short)num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int)num;
		}
		n += number(num, base, field_width, precision, flags);
	}
	return n;
}

/**
 * printf - Format a string and output to debug console.
 * @fmt: The format string to use
 * @...: Arguments for the format string
 */
int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vprintf(fmt, args);
	va_end(args);
	return i;
}
