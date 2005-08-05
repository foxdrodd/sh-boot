#if defined(CONFIG_CPU_SH2)
#define CCR			0xffff8740
#define CCR_CACHE_DISABLE	0x0
#define CCR_CACHE_INIT		0x0
#elif defined(CONFIG_CPU_SUBTYPE_SH7300)
TRA      = 0xffffffd0
EXPEVT   = 0xffffffd4
INTEVT   = 0xA4000000 /* INTEVT2 */
TMU0_TCR = 0xA412FE9C
#define CCR			0xFFFFFFEC
#define CCR_CACHE_DISABLE	0x008		/* Flush the cache, disable */
#define CCR_CACHE_INIT		0x0		/* disable */
#elif defined(CONFIG_CPU_SH3)
TRA      = 0xffffffd0
EXPEVT   = 0xffffffd4
INTEVT   = 0xffffffd8
TMU0_TCR = 0xfffffe9c
#define CCR			0xffffffec	/* Cache Control Register */
#define CCR_CACHE_DISABLE	0x008		/* Flush the cache, disable */
#define CCR_CACHE_INIT		0x0		/* disable */
#elif defined(CONFIG_CPU_SH4)
TRA      = 0xff000020
EXPEVT   = 0xff000024
INTEVT   = 0xff000028
TMU0_TCR = 0xffd80010
#define CCR			0xFF00001C	/* Cache Control Register */
#define CCR_CACHE_DISABLE	0x0808		/* Flush the cache, disable */
#define CCR_CACHE_INIT		0x0		/* disable */
#endif
