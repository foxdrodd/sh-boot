/*
 *
 *  sh-ipl+g/init-adx.S
 *  (based on sh-ipl+g/init-sesh4.S, init-sh2000.S and init-cat68701.S)
 *
 */

#include "config.h"
#include "linkage.h"

ENTRY(init_bsc)

	/* Setup WDT for FRQCR change */
	mov.l	WTCSR_A,r1	/* WTCSR Address */
	mov.w	WTCSR_D,r0	/* WTCSR Data (WDT Disable) */
	mov.w	r0,@r1

	mov.l	WTCNT_A,r1	/* WTCNT Address */
	mov.w	WTCNT_D,r0	/* WTCNT Data (WDT overflows after 5.25msec) */
	mov.w	r0,@r1

	mov.l	FRQCR_A,r1	/* FRQCR Address */
	mov.l	FRQCR_D,r0	/* FRQCR Data */
	mov.w	r0,@r1
	/* Here the clock stops until watchdog timer overflow*/

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

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.l	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */

	/* Wait DRAM refresh 30 times */
	mov	#30,r3
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

/* Configure & Reset Compact flash */
#if defined(CONFIG_DIRECT_COMPACT_FLASH) && defined(CONFIG_IDE)
	/* wait until power on reset complete */
	mov.l	DLY_CNT,r0
2:	dt	r0
	bf	2b

	/* setup PTEA for 16bit attibute access */
	mov.l	PTEA_A,r1	/* PTEA Address */
	mov.l	PTEA_D_ATR,r0	/* 16bit attribute area */
	mov.l	r0,@r1

	/* soft reset */
	mov.l	CF_COR_A, r1
	mov	#0x80, r0
	mov.w	r0, @r1

	/* put some wait */
	mov.l	DLY_CNT2,r0
2:	dt	r0
	bf	2b

	/* soft reset clear */
	mov.l	CF_COR_A, r1
	mov	#0x00, r0
	mov.w	r0, @r1

        mov.l   CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:	mov.w   @r1,r0
        and     #2,r0
        tst     r0,r0
        bt      2b

        mov.l   CF_CCSR_A,r1 /* Powerdown mode bit clear */
        mov     #0,r0
        mov.w   r0,@r1

        mov.l   CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:	mov.w   @r1,r0
        and     #2,r0
        tst     r0,r0
        bt      2b

	/* Select configuration: 0x02, Level interrupt: 0x40 */
	mov.l	CF_COR_A, r1
	mov	#0x42, r0
	mov.w	r0, @r1

        mov.l   CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:
        mov.w   @r1,r0
        and     #2,r0
        tst     r0,r0
        bt      2b

	/* setup PTEA for 16bit I/O access */
	mov.l	PTEA_A,r1	/* PTEA Address */
	mov.l	PTEA_D_IO,r0	/* 16bit I/O area */
	mov.l	r0,@r1
#endif

	rts
	nop

	.align	2

WTCNT_A:
	.long	0xffc00008	/* WDT CNT */
WTCNT_D:
	.word	0x5a00		/* initial counter value, with magic byte "5a" */
WTCSR_A:
	.long	0xffc0000c	/* WDT CSR */
WTCSR_D:	
	.word	0xa507		/* TME = 0, CKS=7, with magic byte "a5" */
FRQCR_A:
	.long	0xFFC00000	/* FRQCR Address */
FRQCR_D:
	.long	0x00000e13	/* 6:2:1 */
BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x0000000d	/* Area 2 & 3 SDRAM, Area 5 & 6 PCMCIA */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	0x1B08		/* Area 1:16bit Area 2:64bit Area 3:64bit 
		                   Area 4:32bit	Area 5:16bit Area 6:8bit */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	0x02720077	/* Device-inter-cycle: 2-wait
				   A6:15 A5:2 A4:2 A3:0 A2:0 A1:15 A0:15 */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	0xFFFE44BF	/* A6: 15 A6B: 7 A5: 15 A5B: 7
				   A4: 15 A3: 2 A2:15 A1:15 A0:15 A0B:7  */
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	0x01717771	/* A6: 0-1 A5: 1-3 A4: 0-1 A3: 1-3
				   A2: 1-3 A1: 1-3 A0: 0-1 */
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
RTCSR_D:
	.long	0xA518		/* RTCSR Write Code A5h Data 18h */

SDMR3_A:
        .long	0xFF900110	/* SDMR2 Address */
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	0x50110014	/* MRSET:'0' */
MCR_D2:
	.long	0x10110014	/* MRSET:'1' */
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */
#if defined(CONFIG_DIRECT_COMPACT_FLASH) && defined(CONFIG_IDE)
PTEA_A:	.long	0xFF000034	/* PTEA Address */
PTEA_D_IO:
	.long	0x00000003	/* SA0-2: 16bit I/O area */
PTEA_D_MEM:
	.long	0x00000005	/* SA0-2: 16bit common memory */
PTEA_D_ATR:
	.long	0x00000007	/* SA0-2: 16bit attribute area */
DLY_CNT:
	.long	80000
DLY_CNT2:
	.long	8000
CF_COR_A:
	.long   (CONFIG_CIS_BASE+0x0200)
CF_CCSR_A:
	.long   (CONFIG_CIS_BASE+0x0202)
CF_PRR_A:
	.long   (CONFIG_CIS_BASE+0x0204)
#endif
