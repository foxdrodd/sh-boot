/* $Id: init-cqsh3.S,v 1.9 2000-05-06 17:02:18+09 gniibe Exp $
 *  Taken from the manual of CQ RISC Evaluatin Kit for SH-3
 *  Copyright (C) 1998 CQ Publishing.
 *
 *  gdb-sh-stub/init-cqsh3.S
 *
 *  Distributed under the permission of CQ Publishing.
 *  Absolutely NO Warranty.
 *
 *  Enhancement:
 *  Copyright (C) 2000 Niibe Yutaka
 *
 */

#include "config.h"
#include "linkage.h"

ENTRY(init_bsc)
	mov.l	BCR1_A,r1	/* BCR1 Address */
	mov.w	BCR1_D,r0	/* BCR1 Data */
	mov.w	r0,@r1

	mov.l	BCR2_A,r1	/* BCR2 Address */
	mov.w	BCR2_D,r0	/* BCR2 Data */
	mov.w	r0,@r1

	mov.l	WCR1_A,r1	/* WCR1 Address */
	mov.w	WCR1_D,r0	/* WCR1 Data */
	mov.w	r0,@r1

	mov.l	WCR2_A,r1	/* WCR2 Address */
	mov.w	WCR2_D,r0	/* WCR2 Data */
	mov.w	r0,@r1

	mov.l	PCR_A,r1	/* PCR Address */
	mov.w	PCR_D,r0	/* PCR Data */
	mov.w	r0,@r1

	mov.l	LED_A,r1	/* LED Address */
	mov	#0,r0		/* LED "0" */
	mov.b	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.w	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.w	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.w	MCR_D,r0	/* MCR Data1 */
	mov.w	r0,@r1

	/* Wait DRAM refresh 8 times */
	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.l	r0,@r1		/* Clear reflesh counter */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

#if defined(CONFIG_COMPACT_FLASH)
	/* Reset Compact flash */
#endif

	rts
	 nop

	.balign 4
LED_A:
	.long	0xa8000000	/* LED Address */
BCR1_A:
	.long	0xffffff60	/* BCR1 Address */
BCR2_A:
	.long	0xffffff62	/* BCR2 Address */
WCR1_A:
	.long	0xffffff64	/* WCR1 Address */
WCR2_A:
	.long	0xffffff66	/* WCR2 Address */
MCR_A:
	.long	0xffffff68	/* MCR Address */
PCR_A:
	.long	0xffffff6c	/* PCR Address */
RTCSR_A:
	.long	0xffffff6e	/* RTCSR Address */
RTCOR_A:
	.long	0xffffff72	/* RTCOR Address */
RFCR_A:
	.long	0xffffff74	/* RFCR Address */
BCR1_D:
	.word	0x0013	/* CS3 = DRAM, CS5/6=PCMCIA  */
BCR2_D:
	.word	0x2bdc	/* CS2/4 = 8bit, CS1/3=32bit, CS5/6=16bit */
WCR1_D:
	.word	0x2aaa	/* Idle wait 2 clock */
WCR2_D:
	.word	0xfe47	/* wait: CS0=10, CS1/2=0, CS3=2, CS4=4, CS5/6=10 */
PCR_D:
	.word	0x0000
RTCSR_D:
	.word	0xa510		/* RTCSR 16mS */
RTCOR_D:
	.word	0xa53a		/* RTCOR 15uS */
MCR_D:
	.word	0x0074		/* DRAM setting 10bit */
RFCR_D:
	.word	0xa400
