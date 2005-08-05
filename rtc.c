#include "config.h"
#include "defs.h"
#include "init.h"

#if defined(CONFIG_CPU_SH2)
#define CMCSR0		0xffff83d2
#define CMCNT0		0xffff83d4
#define CMCOR0		0xffff83d6

#define	CMCSR1		0xffff83d8
#define CMCNT1		0xffff83da
#define CMCOR1		0xffff83dc

#define CPU_CLOCK_QUAR	28
#define R64CNT		CMCNT0

#define RTC_BIT_CHANGE	0x00
#elif defined(CONFIG_CPU_SUBTYPE_SH7300)
//#define TMU_TOCR        0xA412FE90      /* Byte access */ /* not exist */
#define TMU_TSTR        0xA412FE92	/* Byte access */
#define TMU0_TCOR       0xA412FE94	/* Long access */
#define TMU0_TCNT       0xA412FE98	/* Long access */
#define TMU0_TCR        0xA412FE9C	/* Word access */

#elif defined(CONFIG_CPU_SH3)
#define TMU_TOCR        0xfffffe90	/* Byte access */
#define TMU_TSTR        0xfffffe92	/* Byte access */
#define TMU0_TCOR       0xfffffe94	/* Long access */
#define TMU0_TCNT       0xfffffe98	/* Long access */
#define TMU0_TCR        0xfffffe9c	/* Word access */

#elif defined(CONFIG_CPU_SH4)
#define TMU_TOCR        0xffd80000	/* Byte access */
#define TMU_TSTR        0xffd80004	/* Byte access */
#define TMU0_TCOR       0xffd80008	/* Long access */
#define TMU0_TCNT       0xffd8000c	/* Long access */
#define TMU0_TCR        0xffd80010	/* Word access */
#endif

#define TCNTMAX 0xffffffffUL

static unsigned long tick;


static int start_rtc(void)
{
	*(volatile unsigned char *)TMU_TSTR = 0x00;	// stop
	*(volatile unsigned int *)TMU0_TCNT = TCNTMAX;
	*(volatile unsigned int *)TMU0_TCOR = TCNTMAX;
	*(volatile unsigned short *)TMU0_TCR = 0x000004;	// pclk/1024
	*(volatile unsigned char *)TMU_TSTR = 0x01;	// tmu0 start
	return 0;
}

void reset_tick(void)
{
	tick = 0;
	start_rtc();
}

#define TICKS_PER_SEC 128
#define CNT_PER_TICK ((int)(CONFIG_PCLK_FREQ/1024/TICKS_PER_SEC))

unsigned long get_tick(void)
{
#ifdef CONFIG_CPU_SH2
	return p4_inw(CMCNT0);
#else
	unsigned int cnt = TCNTMAX - *(volatile unsigned int *)TMU0_TCNT;
	if (cnt > CNT_PER_TICK) {
		*(volatile unsigned char *)TMU_TSTR = 0x00;	// stop
		*(volatile unsigned int *)TMU0_TCNT =
			TCNTMAX - (cnt % CNT_PER_TICK);
		*(volatile unsigned char *)TMU_TSTR = 0x01;	// start
		tick += cnt / CNT_PER_TICK;
	}
	
	return tick;
#endif
}

#ifdef CONFIG_CPU_SH2
static __init int init_cmt(void)
{
	/* Update every 10ms */
	p4_outw(CMCOR0, (10000 * CPU_CLOCK_QUAR) / 128);

	p4_outw(CMCSR0, 0x41);		/* CKS1=1 (divisor=128), CMIE=1 */
	p4_outw(CMT_CMSTR, 0x01);	/* STR0=1 */

	return 0;
}

__initcall(init_cmt);
#endif

void sleep128(unsigned int count)
{
	int now, start = get_tick();
	do {
		now = get_tick();
	} while (now < start + count && start <= now);
}

int rtc(unsigned int func, unsigned int arg)
{
	switch (func) {
	case 0:		/* Initialize & start */
		return start_rtc();

	case 1:		/* Stop */
	case 2:		/* Set */
	case 3:		/* Get */
		return -1;	/* Not supported yet */

	case 4:		/* Sleep128 */
		sleep128(arg);
		return 0;

	case 5:
		reset_tick();
		return 0;
	case 6:
		return get_tick();

	default:
		return -1;
	}
}
