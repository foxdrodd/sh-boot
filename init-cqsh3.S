/* $Id: init-cqsh3.S,v 1.13 2001/08/09 12:55:58 sugioka Exp $
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
	mov.w	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

	/* Configure & Reset Compact flash */
#ifdef CONFIG_DIRECT_COMPACT_FLASH
# ifdef CONFIG_ETHERNET
	mov.l	ETHER_CIS_A,r1
	!
	mov.w	@r1+,r0
	extu.b	r0,r0
	mov.w	@r1+,r2
	extu.b	r2,r2
	swap.b  r2,r2
	or	r2,r0
	mov.w	@r1+,r2
	extu.b	r2,r2
	swap.w  r2,r2
	or	r2,r0
	mov.w	@r1+,r2
	extu.b	r2,r2
	swap.b  r2,r2
	swap.w  r2,r2
	or	r2,r0
	!
	mov.l	ETHER_CIS_ID,r1	
	cmp/eq	r0,r1
	bf	1f
	!
	mov.l	ETHER_COR_A,r1
	mov.w	ETHER_COR_D1,r0
	mov.w	r0,@r1
	nop
	nop
	nop
	mov.l	ETHER_COR_A,r1
	mov.w	ETHER_COR_D2,r0
	mov.w	r0,@r1
	nop
	!
	mov.l	NE_RESET,r1
	mov.b	@r1,r0
	mov.b	r0,@r1
	!
	mov.l	D8390_P0_ISR,r1
3:
	mov.b	@r1,r0
	tst	#0x80,r0
	bt	3b
	!
	mov	#0xff,r0
	mov.b	r0,@r1
	!
4:	mov.l	0f,r5
	mov.l	0f,r6
	mov.l	0f,r7
	bra	3f
	 nop
	.align	2
0:	.long 0xbabeface
1:
# endif
# ifdef CONFIG_IDE
	mov.l	CF_CIS_A,r1
	!
	mov.w	@r1+,r0
	extu.b	r0,r0
	mov.w	@r1+,r2
	extu.b	r2,r2
	swap.b  r2,r2
	or	r2,r0
	!
	mov.l	CF_CIS_ID,r1	
	cmp/eq	r0,r1
	bf	3f
	!
	mov.l	CF_COR_A,r1
	mov.w	CF_COR_D1,r0
	mov.w	r0,@r1
	nop
	nop
	nop
	mov.l	CF_COR_A,r1
	mov.w	CF_COR_D2,r0
	mov.w	r0,@r1
	mov.l	0f,r5
	mov.l	0f,r6
	mov.l	0f,r7
	bra	3f
	 nop
	.align	2
0:	.long 0xbabeface
# endif
3:
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
#ifdef CONFIG_DIRECT_COMPACT_FLASH
# ifdef CONFIG_ETHERNET
ETHER_CIS_A:	.long	(CONFIG_CIS_BASE+0x001c)
ETHER_CIS_ID:
	.long	0xc1ab0149	/* manfid 0x0149, 0xc1ab */
NE_RESET:
	.long	(CONFIG_IO_BASE+0x0300+0x001f)
D8390_P0_ISR:
	.long	(CONFIG_IO_BASE+0x0300+0x0007)
	!
/* Setup CIS/COR: This is only valid for: Planex Communications Inc. */
/* Use dump_cid(8) to get information of your card */
ETHER_COR_A:
	.long	(CONFIG_CIS_BASE+0x3f8)
ETHER_COR_D1:
	.word	0x00E0  /* Reset, configuration: 0x20, Level interrupt: 0x40 */
ETHER_COR_D2:
	.word	0x0060  /* Select configuration: 0x20, Level interrupt: 0x40 */
# endif
# ifdef CONFIG_IDE
CF_CIS_A:	.long	(CONFIG_CIS_BASE+0x0074)
CF_CIS_ID:
	.long	0x00000101	/* disk_interface [ide] */
CF_COR_A:
	.long	(CONFIG_CIS_BASE+0x200)
CF_COR_D1:
	.word	0x00C2  /* Reset, configuration: 0x02, Level interrupt: 0x40 */
CF_COR_D2:
	.word	0x0042  /* Select configuration: 0x20, Level interrupt: 0x40 */
# endif
#endif
BCR1_D:
	.word	0x0013	/* CS3 = DRAM, CS5/6=PCMCIA  */
BCR2_D:
	.word	0x2bdc	/* CS2/4 = 8bit, CS1/3=32bit, CS5/6=16bit */
WCR1_D:
	.word	0x2aaa	/* Idle wait 2 clock */
WCR2_D:
	.word	0xfe47	/* wait: CS0=10, CS1/2=0, CS3=2, CS4=4, CS5/6=10 */
PCR_D:
	.word	0x0050
RTCSR_D:
	.word	0xa510		/* RTCSR 16mS */
RTCOR_D:
	.word	0xa53a		/* RTCOR 15uS */
MCR_D:
	.word	0x0074		/* DRAM setting 10bit */
RFCR_D:
	.word	0xa400
