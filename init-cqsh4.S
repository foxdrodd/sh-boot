/* $Id: init-cqsh4.S,v 1.16 2001/02/14 09:51:52 gniibe Exp $
 * Taken from CQ RISC Evaluatin Kit for SH-4 "tool/rom/start.c"
 * Copyright (C) 1999 CQ Publishing.
 *
 *  gdb-sh-stub/init-cqsh4.S
 *
 *  Distributed under the permission of CQ Publishing.
 *  Absolutely NO Warranty.
 *
 */

#include "config.h"
#include "linkage.h"
ENTRY(init_bsc)
	mov.l	FRQCR_A,r1	/* FRQCR Address */
	mov.l	FRQCR_D,r0	/* FRQCR Data */
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
	mov	#0xaa,r0
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
	mov	#0xaa,r0
	mov.b	r0,@r1

#ifdef CONFIG_CQ_BRIDGE
	mov.l	BRIDGE_ENABLE_A,r1
	mov.w	BRIDGE_ENABLE_CURSES,r0
	mov.w	r0,@r1
	mov.w	r0,@r1
	mov.w	r0,@r1
	mov.w	r0,@r1
	!
	mov.l	BRIDGE_ISA_INTR_LVL_A,r1
	mov	#0,r0
	mov.w	r0,@r1
	mov.l	BRIDGE_ISA_INTR_MASK_A,r1
	mov.w	r0,@r1
	!
	mov.l	BRIDGE_ISA_CTRL_A,r1
	mov.w	LOOP_NUMBER,r2
	tst	r2, r2
1:	mov.w	r0,@r1
	bf/s	1b
	 dt	r2	
	!
	mov.l	BRIDGE_ISA_CTRL_D,r0
	mov.w	LOOP_NUMBER,r2
	tst	r2, r2
1:	mov.w	r0,@r1
	bf/s	1b
	 dt	r2	
	!
	mov	#0xff,r0
	mov.l	BRIDGE_ISA_INTR_STAT_A,r1
	mov.w	r0,@r1
#endif
	rts
	 nop

	.align	2
FRQCR_A:
	.long	0xFFC00000	/* FRQCR Address */
FRQCR_D:
	.long	0x00000e13	/* 6:2:1 */
BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x0020000C	/* Area 2/3 SDRAM, bit21:1 */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0xB5F8		/* Area 0:16bit Area 2:32bit Area 4/5:8bit b3/2:	10 (Area1=16bit) */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x77117121	/* Area 0/2/4/5: Idle wait 1 clock, bit6/5/4:010 */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xFC8AE520	/* Area 0: Wait 6 Area 4:9 Area 5:1 Area2:SDRAM CL=2, Area1=bit8/7/6:100 (wait 6) */
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x07007000	/* Area 0/1/2/4/5 Holdwait 0 clock, Area1=bit6:0 bit5/4:00 */
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
	.long	0x10192194	/* MRSET:'0' */
MCR_D2:
	.long	0x50192194	/* MRSET:'1' */
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */
#ifdef CONFIG_CQ_BRIDGE
BRIDGE_ENABLE_A:
	.long	0xA4000000
BRIDGE_ISA_INTR_LVL_A:
	.long	0xA400002A
BRIDGE_ISA_INTR_MASK_A:
	.long	0xA400002C
BRIDGE_ISA_CTRL_A:
	.long	0xA4000028
BRIDGE_ISA_INTR_STAT_A:
	.long	0xA400002E
BRIDGE_ISA_CTRL_D:
	.long	0x00008000
LOOP_NUMBER:
	.word	10000
BRIDGE_ENABLE_CURSES:
	.word	0x4949
#endif
