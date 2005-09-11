/* $Id: entry.S,v 1.33 2001/08/09 12:55:58 sugioka Exp $
 *
 * sh-ipl+g/entry.S
 *
 * Some code was taken from sh-stub.c of GDB 4.18.
 *
 *  Copyright (C) 1999, 2000  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#include "config.h"
#include "linkage.h"
#include "regs.h"
	
	.text
	.balign 	4096,0,4096

ENTRY(start)
   	/*
	 * This code executes at powerup and manual reset,
	 * at location 0xa0000000 (P2) with the MMU disabled.
	 */
#ifndef CONFIG_CPU_SH2	
	mov.l	8f,r0
	mov.l	@r0,r13			/* save EXPEVT */
#endif
	/* Disable cache */
	mov.l	CCR_A, r1		/* CCR Address */
	mov.l	CCR_DISABLE, r0	/* CCR Data */
	mov.l	r0, @r1

	/* Initialize BSC */
	mov.l	1f, r0
	jsr	@r0
	 nop

	/* Initialize cache */
	mov.l	CCR_A, r1	/* CCR Address */
	mov.l	CCR_D, r0	/* CCR Data */
	mov.l	r0, @r1

	/* Initialize VBR */
	/* While the initial code executes from P2, the new
	 * value of the VBR points to P2 (0xA0000000) or P1 (0x80000000) which
	 * is where the stub code is linked.  This ensures that
	 * the exception handlers can run regardless of the
	 * setting of the MMU and cache when the trap occurs.
	 * This enables traps into the stub from the kernel
	 * to work in the brief window between initialising
	 * the MMU and initialising the kernel's trap handlers.
	 */
	mov.l	VBR_INIT, r0
	ldc	r0, vbr

	/* SR: MD=1, BL=1, RB=0, IMASK=0xf */
	mov.l	INITIAL_SR, r0
	ldc	r0, sr

	mov.l	INITIAL_STACK, r0
	mov	r0, r15

	/* Initialize BSS */
	mov.l	2f, r1
	add	#4, r1
	mov.l	3f, r2
	mov	#0, r0
9:	cmp/hs	r2, r1
	bf/s	9b		! while (r1 < r2)
	 mov.l	r0, @-r2

	/* Move data section on ram */
	mov.l	4f, r1
	mov.l	5f, r2
	mov.l	11f, r3
10:	mov.l	@r3+, r0
	mov.l	r0, @r1
	add	#4,r1
	cmp/hs	r2, r1
	bf	10b		! while (r1 < r2)	
#if !defined(CONFIG_CPU_SH2)	
	mov.l	7f,r0
	mov.l	r13,@r0		/* set initial EXPEVT */
#endif
	/* Jump to GDB Stub */
	mov.l	6f, r0
	jmp	@r0
	 nop

	.align	2
1:	.long	SYMBOL_NAME(init_bsc)
2:	.long	SYMBOL_NAME(__bss_start)
3:	.long	SYMBOL_NAME(__bss_end)
4:	.long	SYMBOL_NAME(__data_start)
5:	.long	SYMBOL_NAME(__data_end)
6:	.long	SYMBOL_NAME(start_main)
7:	.long	SYMBOL_NAME(expevt_on_start)
#if !defined(CONFIG_CPU_SH2)	
8:	.long	EXPEVT
#endif	
11:	.long   SYMBOL_NAME(_etext)
	
CCR_A:	.long	CCR
CCR_DISABLE:
	.long	CCR_CACHE_DISABLE
CCR_D:	.long	CCR_CACHE_INIT
INITIAL_SR:
	.long	0x500000f0	/* MD=1, RD=0, BL=1, IMASK=0xf */
INITIAL_STACK:
	.long	SYMBOL_NAME(stub_stack)
VBR_INIT:
	.long	SYMBOL_NAME(vbr_base)

/* End of entry.S */
