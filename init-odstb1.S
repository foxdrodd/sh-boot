/* $Id: init-odstb1.S,v 1.1 2001/06/07 04:47:07 sugioka Exp $
 *
 *  gdb-sh-stub/init-osstb1.S
 *
 * Normally the Overdrive is booted from the HUDI, so no BSC programming
 * is needed.
 * One day this will probably change...
 */

#include "linkage.h"
ENTRY(init_bsc)
	rts
	 nop
