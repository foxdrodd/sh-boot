/* $Id: init-cqsh4.S,v 1.9 2000-05-06 17:01:58+09 gniibe Exp $
 * Taken from CQ RISC Evaluatin Kit for SH-4 "tool/rom/start.c"
 * Copyright (C) 1999 CQ Publishing.
 *
 *  gdb-sh-stub/init-cqsh4.S
 *
 *  Distributed under the permission of CQ Publishing.
 *  Absolutely NO Warranty.
 *
 */

#include "linkage.h"
ENTRY(init_bsc)
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

	mov.l	LED_A,r1	/* LED Address */
	mov	#0,r0		/* LED "0" */
	mov.b	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D1,r0	/* MCR Data1 */
	mov.l	r0,@r1

	mov.l	SDMR2_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	/* Wait DRAM refresh 8 times */
	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D2,r0	/* MCR Data2 */
	mov.l	r0,@r1

	mov.l	SDMR2_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

#if 1	/* Dump registers to 0xa8001200 */
	mov.l	DUMP_A, $r0
	mov.l	$r15, @-$r0
	mov.l	$r14, @-$r0
	mov.l	$r13, @-$r0
	mov.l	$r12, @-$r0
	mov.l	$r11, @-$r0
	mov.l	$r10, @-$r0
	mov.l	$r9, @-$r0
	mov.l	$r8, @-$r0
	mov.l	$r7, @-$r0
	mov.l	$r6, @-$r0
	mov.l	$r5, @-$r0
	mov.l	$r4, @-$r0
	stc.l	$ssr, @-$r0
	sts.l	$pr,  @-$r0
	stc.l	$spc, @-$r0
#endif
	rts
	 nop

	.balign 4
DUMP_A:
	.long	0xa8001200 + 15*4

BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x0000000C	/* Area 2/3 SDRAM */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0xB5F8		/* Area 0/1:16bit Area 2:32bit Area 4/5:8bit */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x77117111	/* Area 0/1/2/4/5: Idle wait 1 clock */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xFC8AE520	/* Area 0/1: Wait 6 Area 4:9 Area 5:1 Area2:SDRAM CL=2 */
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x07117111	/* Area 0/1/2/4/5 Holdwait 1 clock */
LED_A:
	.long	0xB4000000	/* LED Address */
RTCOR_A:
	.long	0xFF800024	/* RTCOR Address */
RTCOR_D:
	.long	0xA541		/* RTCOR Write Code A5h Data 41h */
RTCSR_A:
	.long	0xFF80001C	/* RTCSR Address */
RTCSR_D:
	.long	0xA510		/* RTCSR Write Code A5h Data 10h*/
SDMR2_A:
	.long	0xFF90008C	/* SDMR2 Address */
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	0x00192194	/* MRSET:'0' */
MCR_D2:
	.long	0x40192194	/* MRSET:'1' */
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */
