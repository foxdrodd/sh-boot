/* $Id: init-apsh4.S,v 1.1 2000-05-25 15:55:53+09 gniibe Exp $
 * Taken from CQ RISC Evaluatin Kit for SH-4 "tool/rom/start.c"
 * Copyright (C) 1999 CQ Publishing.
 *
 * Modified for AP-SH4 by KOMORIYA Takeru 5/15, 2000
 *
 *  gdb-sh-stub/init-apsh4.S
 *
 *  Distributed under the permission of CQ Publishing.
 *  Absolutely NO Warranty.
 *
 */

#include "linkage.h"
ENTRY(init_bsc)
	/* Parallel I/O Port */
	mov.l	PCTRB_A,r1
	mov.l	PCTRB_D,r0
	mov.l	r0,@r1

	/* disable SCIF */
	mov.l	SCSCR_A,r1
	mov		#0,r0
	mov.w	r0,@r1

	/* disable TMU0 */
	mov.l	TSTR_A,r1
	mov		#0,r0
	mov.b	r0,@r1

	mov.l	TCR0_A,r1
	mov		#0,r0
	mov.w	r0,@r1
		
	mov.l	BCR1_A,r1	/* BCR1 Address */
	mov.l	BCR1_D,r0	/* BCR1 Data */
	mov.l	r0,@r1

	mov.l	BCR2_A,r1	/* BCR2 Address */
	mov.l	BCR2_D,r0	/* BCR2 Data */
	mov.w	r0,@r1

	mov.l	WCR1_A,r1	/* WCR1 Address */
	mov.l	WCR1_D,r0	/* WCR1 Data */
	mov.l	r0,@r1

	mov.l	WCR2_A,r1	/* WCR2 Address */
	mov.l	WCR2_D,r0	/* WCR2 Data */
	mov.l	r0,@r1

	mov.l	WCR3_A,r1	/* WCR3 Address */
	mov.l	WCR3_D,r0	/* WCR3 Data */
	mov.l	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D1,r0	/* MCR Data1 (MRSET=0) */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	/* Wait DRAM refresh 30 times */
	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */
	mov	#30,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b
	nop

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D2,r0	/* MCR Data2 (MRSET=1)*/
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	PDTRB_A,r1	/* Turn on LED */
	mov	#8,r0
	mov.w	r0,@r1

	rts
	nop

	.balign 4

BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x0000000C	/* Area 2,3 SDRAM */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0xAAF9		/* Area 0,1,4,5,6:16bit Area 2,3:32bit */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x77770711  /* Wait control register */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xFFFE4E48
!	.long	0xFFFE4F20
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x07777755
PCTRB_A:				/* PORT B Control Reg. */
	.long   0xff800040
PCTRB_D:
	.long   0x000000c0
PDTRB_A:				/* PORT B (LED) */
	.long   0xff800044
RTCOR_A:
	.long	0xFF800024	/* RTCOR Address */
RTCOR_D:
	.long	0xA540		/* RTCOR Write Code A5h Data 40h */
RTCSR_A:
	.long	0xFF80001C	/* RTCSR Address */
RTCSR_D:
	.long	0xA510		/* RTCSR Write Code A5h Data 10h*/
SDMR3_A:
	.long	0xFF94008c	/* SDMR3 Address */
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	0x18094194	/* MRSET:'0' */
MCR_D2:
	.long	0x58094194	/* MRSET:'1' */
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */
SCSCR_A:
	.long	0xFFE80008	/* SCSCR Address */
TSTR_A:
	.long	0xFFD80004	/* TSTR Address */
TCR0_A:
	.long	0xFFD80010	/* TCR0 Address */
