/* $Id: init-odstb1.S,v 1.2 2003/12/12 15:34:43 lethal Exp $
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
