/* $Id: init-sesh4.S,v 1.10 2000-05-09 10:31:44+09 gniibe Exp $
 *
 *  gdb-sh-stub/init-sesh4.S
 *
 *  Data taken from Chapter 11 of "SH7750 SolutionEngine Manual"
 *
 */

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
	mov	#0x0f,r0	/* LED ALL 'on' */
	shll8	r0
	mov.w	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D1,r0	/* MCR Data1 */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	! Do you need PCMCIA setting?	
	! If so, please add the lines here...

	mov.l	RTCNT_A,r1	/* RTCNT Address */
	mov.l	RTCNT_D,r0	/* RTCNT Data */
	mov.w	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D1,r0	/* RTCSR Data */
	mov.w	r0,@r1

	/* Wait DRAM refresh 8 times */
	mov.l	RFCR_A,r1	/* RFCR Address */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D2,r0	/* MCR Data2 */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	RTCSR_A,r0
	mov.l	RTCSR_D2,r1
	mov.w	r1,@r0

	rts
	 nop

	.balign 4

FRQCR_A:
	.long	0xFFC00000	/* FRQCR Address */
FRQCR_D:
	.long	0x00000e13	/* 6:2:1 */
BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x00000008	/* Area 3 SDRAM */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0x2E3C		/* Area 1:32bit Area 2:32bit Area 4:16bit 
		                   Area 3:64bit	Area 5:32bit Area 6:16bit */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x02720777	/* Device-inter-cycle: 2-wait
				   A6:15 A5:2 A4:2 A3:0 A2:15 A1:15 A0:15 */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xFFFE4FFF	/* A6: 15 A6B: 7 A5: 15 A5B: 7
				   A4: 15 A3: 2 A2:15 A1:15 A0:15 A0B:7  */
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x01717771	/* A6: 0-1 A5: 1-3 A4: 0-1 A3: 1-3
				   A2: 1-3 A1: 1-3 A0: 0-1 */
LED_A:
	.long	0xB0C00000	/* LED Address */

RTCNT_A:
	.long	0xFF800020	/* RTCNT Address */
RTCNT_D:
	.long	0xA500		/* RTCNT Write Code A5h Data 00h */

RTCOR_A:
	.long	0xFF800024	/* RTCOR Address */
RTCOR_D:
	.long	0xA510		/* RTCOR Write Code A5h Data 10h (15us) */

RTCSR_A:
	.long	0xFF80001C	/* RTCSR Address */
RTCSR_D1:
	.long	0xA510		/* RTCSR Write Code A5h Data 10h */
RTCSR_D2:
	.long	0xA518		/* RTCSR Write Code A5h Data 18h */

SDMR3_A:
	.long	0xFF940110	/* SDMR3 Address */
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	0x8801001C	/* MRSET:'0' */
MCR_D2:
	.long	0xC801001C	/* MRSET:'1' */
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */

