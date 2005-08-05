#ifndef	__OSDEP_H__
#define __OSDEP_H__

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#if defined(__BIG_ENDIAN__)
#define ntohl(x) (x)
#define htonl(x) (x)
#define ntohs(x) (x)
#define htons(x) (x)
#else				/* must be LITTLE_ENDIAN */
#define ntohl(x) swab32(x)
#define htonl(x) swab32(x)
#define ntohs(x) swab16(x)
#define htons(x) swab16(x)
#endif				/* LITTLE_ENDIAN */

static __inline__ __const__ unsigned long int swab32(unsigned long int x)
{
      __asm__("swap.b	%0, %0\n\t" "swap.w %0, %0\n\t" "swap.b %0, %0":"=r"(x)
      :	"0"(x));
	return x;
}

static __inline__ __const__ unsigned short int swab16(unsigned short int x)
{
      __asm__("swap.b %0, %0":"=r"(x)
      :	"0"(x));
	return x;
}

#endif

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
