#include "linkage.h"

FRQCR	= 0xffffff80

BSC	= 0xffffff60
BCR1	= 0xffffff60
BCR2	= 0xffffff62
BCR3	= 0xffffff7e
WCR1	= 0xffffff64
WCR2	= 0xffffff66
MCR	= 0xffffff68
DCR	= 0xffffff6a
RTCSR	= 0xffffff6e
RTCNT	= 0xffffff70
RTCOR	= 0xffffff72
RFCR	= 0xffffff74

ENTRY(init_bsc)
	mov	#FRQCR,r1
	mov.w	FRQCR_D,r0
	mov.w	r0,@r1

	mov.l	PECR_A,r1
	mov.w	PECR_D,r0
	mov.w	r0,@r1

	mov.l	BSC_A,r0
	ldc	r0,gbr
	
	mov.w	BCR1_D,r0
	mov.w	r0,@(BCR1-BSC,gbr)

	mov.w	BCR2_D,r0
	mov.w	r0,@(BCR2-BSC,gbr)

	mov.w	BCR3_D,r0
	mov.w	r0,@(BCR3-BSC,gbr)

	mov.w	WCR1_D,r0
        mov.w   r0,@(WCR1-BSC,gbr)

	mov.w	WCR2_D,r0
        mov.w   r0,@(WCR2-BSC,gbr)

	mov.l	LED_A,r1
	mov	#0x0f,r0	/* LED half 'on' */
	shll8	r0
	mov.w	r0,@r1

        mov.w   MCR_D,r0
        mov.w   r0,@(MCR-BSC,gbr)

	mov.w	DCR_D,r0
        mov.w   r0,@(DCR-BSC,gbr)

        mov.w   RTCSR_D,r0
        mov.w   r0,@(RTCSR-BSC,gbr)
	
	mov.w	RTCNT_D,r0
        mov.w   r0,@(RTCNT-BSC,gbr)

        mov.w   RTCOR_D,r0
        mov.w   r0,@(RTCOR-BSC,gbr)

        mov	#0xa4,r0
	shll8	r0
        mov.w   r0,@(RFCR-BSC,gbr)	/* Clear reflesh counter */
	
        mov	#8,r1			/* Wait DRAM refresh 8 times */
1:	mov.w   @(RFCR-BSC,gbr),r0
        cmp/hi  r1,r0
        bf      1b

	rts
	 nop

	.align 2

LED_A:		.long	0xb1800000	/* LED Address */
PECR_A:		.long	0xa4000108
BSC_A:		.long	BSC

FRQCR_D:	.word	0x0122
BCR1_D:		.word	0x0810
BCR2_D:		.word	0x2ef0
BCR3_D:		.word	0x0000
WCR1_D:		.word	0x0c30
WCR2_D:		.word	0x5d5a
MCR_D:		.word	0x0055
DCR_D:		.word	0x0000
RTCSR_D:	.word	0xa500+0x08
RTCNT_D:	.word	0xa500+0x00
RTCOR_D:	.word	0xa500+0x4e
PECR_D:		.word	0xaa8a

