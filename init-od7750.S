/* $Id: init-od7750.S,v 1.2 2003/12/12 15:34:43 lethal Exp $
 *
 *  gdb-sh-stub/init-od7750.S
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



	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D1,r0	/* MCR Data1 */
	mov.l	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D1,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	RTCNT_A,r1	/* RTCNT Address */
	mov.l	RTCNT_D1,r0	/* RTCNT Data */
	mov.w	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D1,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1



	mov.l	MCR_A,r1	/* MCR Address */
	mov.l	MCR_D2,r0	/* MCR Data2 */
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	RTCNT_A,r1	/* RTCNT Address */
	mov.l	RTCNT_D2,r0	/* RTCNT Data */
	mov.w	r0,@r1

	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D2,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */

	mov.l	RTCSR_A,r0
	mov.l	RTCSR_D2,r1
	mov.w	r1,@r0



	mov.l	FRQCR_A,r1	/* FRQCR Address */
	mov.l	FRQCR_D,r0	/* FRQCR Data */
	mov.w	r0,@r1

	mov.l	RS232_A,r0
	mov.l	RS232_D,r1
	mov.l	r1,@r0

	rts
	 nop

	.balign 4

FRQCR_A:
	.long	0xFFC00000	/* FRQCR Address */
FRQCR_D:
	.long	0x00000e23
BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x000ac008
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0xff3c
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x37773774
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xfffe6fef
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x07777775

RTCNT_A:
	.long	0xFF800020	/* RTCNT Address */
RTCNT_D1:
	.long	0xA500
RTCNT_D2:
	.long	0xA500

RTCOR_A:
	.long	0xFF800024	/* RTCOR Address */
RTCOR_D:
	.long	0xA570

RTCSR_A:
	.long	0xFF80001C	/* RTCSR Address */
RTCSR_D1:
	.long	0xa508
RTCSR_D2:
	.long	0xa508

SDMR3_A:
	.long	0xFF940190	/* SDMR3 Address */
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	0x080a6214
MCR_D2:
	.long	0x480a6214
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D1:
	.long	0xA400
RFCR_D2:
	.long	0xA400
RS232_A:
	.long	0xa3000000
RS232_D:
	.long	0x0000001e
	

