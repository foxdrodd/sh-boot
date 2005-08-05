/* $Id: io.h,v 1.14 2001/06/04 09:09:19 gniibe Exp $
 *
 * gdb-sh-stub/io.h
 *
 *  Copyright (C) 1999  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#ifndef _IO_H
#define _IO_H	1

extern __inline__ void cli(void)
{
	unsigned long __dummy;
	__asm__ __volatile__("stc	sr,%0\n\t"
			     "or	%1,%0\n\t" "ldc	%0,sr":"=&z"(__dummy)
			     :"r"(0x10000000)
			     :"memory");
}

extern __inline__ void sti(void)
{
	unsigned long __dummy;

	__asm__ __volatile__("stc	sr,%0\n\t"
			     "and	%1,%0\n\t" "ldc	%0,sr":"=&z"(__dummy)
			     :"r"(0xefffffff)
			     :"memory");
}

extern __inline__ unsigned long p4_inb(unsigned long addr)
{
	return *(volatile unsigned char *)addr;
}

extern __inline__ unsigned long p4_inw(unsigned long addr)
{
	return *(volatile unsigned short *)addr;
}

extern __inline__ unsigned long p4_inl(unsigned long addr)
{
	return *(volatile unsigned long *)addr;
}

extern __inline__ void p4_outb(unsigned long addr, unsigned short b)
{
	*(volatile unsigned char *)addr = b;
}

extern __inline__ void p4_outw(unsigned long addr, unsigned short b)
{
	*(volatile unsigned short *)addr = b;
}

extern __inline__ void p4_outl(unsigned long addr, unsigned int b)
{
	*(volatile unsigned long *)addr = b;
}

#define p4_in(addr)	*(addr)
#define p4_out(addr,data) *(addr) = (data)

/* Following copied from linux (include/asm-sh/byteorder.h) */
static __inline__ unsigned long swab32(unsigned long x)
{
      __asm__("swap.b	%0, %0\n\t" "swap.w %0, %0\n\t" "swap.b %0, %0":"=r"(x)
      :	"0"(x));
	return x;
}

static __inline__ unsigned short swab16(unsigned short x)
{
      __asm__("swap.b %0, %0":"=r"(x)
      :	"0"(x));
	return x;
}

#ifdef CONFIG_LITTLE_ENDIAN
#define le16_to_cpu(n) (u16)(n)
#define le32_to_cpu(n) (u32)(n)
#define be16_to_cpu(n) (u16)swab16((unsigned short)(n))
#define be32_to_cpu(n) (u32)swab32((unsigned long)(n))
#else				/* !CONFIG_LITTLE_ENDIAN */
#define le16_to_cpu(n) (u16)swab16((unsigned short)(n))
#define le32_to_cpu(n) (u32)swab32((unsigned long)(n))
#define be16_to_cpu(n) (u16)n
#define be32_to_cpu(n) (u32)n
#endif				/* !CONFIG_LITTLE_ENDIAN */

#if defined(CONFIG_IDE)
#if defined(CONFIG_DIRECT_COMPACT_FLASH)
#define IDE_PORT_SHIFT  0
#elif defined(CONFIG_SOLUTION_ENGINE)
#if defined(CONFIG_MRSHPC)
#define IDE_PORT_SHIFT  0
#else
#define IDE_PORT_SHIFT  1
#endif
#elif defined(CONFIG_CQ_BRIDGE)
#define IDE_PORT_SHIFT  0
#endif

extern unsigned long ide_offset;

extern void delay(void);

extern __inline__ unsigned long ide_inb(unsigned short port)
{
	unsigned long addr = ide_offset + (port << IDE_PORT_SHIFT);
	unsigned long v;

	if (IDE_PORT_SHIFT)
		v = (*(volatile unsigned short *)addr) & 0xff;
	else
		v = *(volatile unsigned char *)addr;

	delay();
	return v;
}

extern __inline__ unsigned long ide_inw(unsigned short port)
{
	unsigned long addr = ide_offset + (port << IDE_PORT_SHIFT);
	unsigned long v = *(volatile unsigned short *)addr;

	delay();
	return v;
}

extern __inline__ void ide_outb(unsigned long b, unsigned short port)
{
	unsigned long addr = ide_offset + (port << IDE_PORT_SHIFT);

	if (IDE_PORT_SHIFT)
		*(volatile unsigned short *)addr = b;
	else
		*(volatile unsigned char *)addr = b;
	delay();
}

extern __inline__ void ide_outw(unsigned long b, unsigned short port)
{
	unsigned long addr = ide_offset + (port << IDE_PORT_SHIFT);

	*(volatile unsigned short *)addr = b;
	delay();
}
#endif
#endif				/* _IO_H */
