/*
 *	$Id: init-hp600.S,v 1.1 2000-05-28 18:48:38+09 gniibe Exp $
 *	gdb-sh-stub/init-hp600.S
 */

#include "linkage.h"
ENTRY(init_bsc)

	! Disable MMU...		
	! Yes, it should be done in shlo.exe. 
	! This code will be removed soon.
	mov	#4,r0
	mov.l	MMUCR_A,r1
	mov.l	r0,@r1

	rts
	 nop

	.balign 4
MMUCR_A:	
	.long	0xffffffe0

