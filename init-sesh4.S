/* $Id: init-sesh4.S,v 1.18 2001/06/29 13:44:47 gniibe Exp $
 *
 *  gdb-sh-stub/init-sesh4.S
 *
 *  Data taken from Chapter 11 of "SH7750 SolutionEngine Manual"
 *  Modified for SH7751 based on manual and Hitachi FW defaults.
 */

#include "config.h"
#include "linkage.h"

#ifdef CONFIG_CPU_SUBTYPE_SH7751
#define BCR2_D_VALUE	0x2FFC	   /* Area 1-6 width: 32/32/32/32/32/16 */
#define WCR1_D_VALUE    0x02770771 /* DMA:0 A6:2 A3:0 A0:1 Others:15 */
#ifdef CONFIG_MRSHPC
#define WCR2_D_VALUE    0xFFFE4FE7 /* A6:15 A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:6  A0B:7  */
#else /* CONFIG_MRSHPC*/
#define WCR2_D_VALUE    0x7FFE4FE7 /* A6:3  A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:6  A0B:7  */
#endif /* CONFIG_MRSHPC */
#define WCR3_D_VALUE	0x01777771 /* A6: 0-1 A5: 1-3 A4: 1-3 A3: 1-3
				      A2: 1-3 A1: 1-3 A0: 0-1 */
#define LED_ADDRESS	0xBA000000 /* Address of LED register */ 	
#define RTCOR_D_VALUE	0xA50D	   /* Write code A5, data 0D (~15us?) */
#define SDMR3_ADDRESS	0xFF940088 /* SDMR3 address on 32-bit bus */
#define MCR_D1_VALUE	0x100901B4 /* SDRAM 32-bit, CAS/RAS Refresh, ... */
#define MCR_D2_VALUE	0x500901B4 /* Same w/MRSET now 1 (mode reg cmd) */
#define SWITCH_ADDR	0xB9000000 /* Address of DIP switches */
#else /* !CONFIG_CPU_SUBTYPE_SH7751 */
#define BCR2_D_VALUE	0x2E3C	   /* Area 1-6 width: 32/32/64/16/32/16 */
#define WCR1_D_VALUE	0x02720777 /* DMA:0 A6:2 A4:2 A3:0 Others:15 */
#define WCR2_D_VALUE	0xFFFE4FFF /* A6:15 A6B:7 A5:15 A5B:7 A4:15
				      A3:2  A2:15 A1:15 A0:15 A0B:7  */
#define WCR3_D_VALUE	0x01717771 /* A6: 0-1 A5: 1-3 A4: 0-1 A3: 1-3
				      A2: 1-3 A1: 1-3 A0: 0-1 */
#define LED_ADDRESS	0xB0C00000 /* Address of LED register */ 	
#define RTCOR_D_VALUE	0xA510	   /* Write code A5, data 10 (~15us?) */
#define SDMR3_ADDRESS	0xFF940110 /* SDMR3 address on 64-bit bus */
#define MCR_D1_VALUE	0x8801001C /* SDRAM 64-bit, CAS/RAS Refresh, ... */
#define MCR_D2_VALUE	0xC801001C /* Same w/MRSET now 1 (mode reg cmd) */
#define SWITCH_ADDR	0xb0800000 /* Address of DIP switches */
#endif /* !CONFIG_CPU_SUBTYPE_SH7751 */


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
	mov	#0xff,r0	/* LED ALL 'on' */
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

	mov.l	BOOT_CONFIG_WORD,r1
	mov.w	@r1,r0
	exts.b	r0,r0
	cmp/eq	#0xde,r0	! MAGIC # for "doDEs"
	bf	1f
	mov.l	0f,r5
	mov.l	0f,r6
	mov.l	0f,r7
	bra	1f
	 nop
	.align	2
0:	.long 0xbabeface
1:
	rts
	 nop

	.align	2

FRQCR_A:
	.long	0xFFC00000	/* FRQCR Address */
FRQCR_D:
#ifdef CONFIG_CPU_SUBTYPE_SH_R
	.long	0x00000e1a	/* 12:3:3 */
#else
#ifdef CONFIG_GOOD_SESH4
	.long	0x00000e13	/* 6:2:1 */
#else
	.long	0x00000e23	/* 6:1:1 */
#endif
#endif	/* CONFIG_CPU_SUBTYPE_SH_R */
BCR1_A:
	.long	0xFF800000	/* BCR1 Address */
BCR1_D:
	.long	0x00000008	/* Area 3 SDRAM */
BCR2_A:
	.long	0xFF800004	/* BCR2 Address */
BCR2_D:
	.long	BCR2_D_VALUE	/* Bus width settings */
WCR1_A:
	.long	0xFF800008	/* WCR1 Address */
WCR1_D:
	.long	WCR1_D_VALUE	/* Inter-area or turnaround wait states */
WCR2_A:
	.long	0xFF80000C	/* WCR2 Address */
WCR2_D:
	.long	WCR2_D_VALUE	/* Per-area access and burst wait states */
WCR3_A:
	.long	0xFF800010	/* WCR3 Address */
WCR3_D:
	.long	WCR3_D_VALUE	/* Address setup and data hold cycles */
LED_A:
	.long	LED_ADDRESS	/* LED Address */

RTCNT_A:
	.long	0xFF800020	/* RTCNT Address */
RTCNT_D:
	.long	0xA500		/* RTCNT Write Code A5h Data 00h */

RTCOR_A:
	.long	0xFF800024	/* RTCOR Address */
RTCOR_D:
	.long	RTCOR_D_VALUE	/* Set refresh time (about 15us) */

RTCSR_A:
	.long	0xFF80001C	/* RTCSR Address */
RTCSR_D:
	.long	0xA518		/* RTCSR Write Code A5h Data 18h */

SDMR3_A:
	.long	SDMR3_ADDRESS
MCR_A:
	.long	0xFF800014	/* MCR Address */
MCR_D1:
	.long	MCR_D1_VALUE
MCR_D2:
	.long	MCR_D2_VALUE
RFCR_A:
	.long	0xFF800028	/* RFCR Address */
RFCR_D:
	.long	0xA400		/* RFCR Write Code A4h Data 00h */
BOOT_CONFIG_WORD:
	.long	SWITCH_ADDR
