/*
 *	$Id: sh-stub-boot.S,v 1.1 2001/06/07 04:47:07 sugioka Exp $
 *	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>
 */

	.text

	.globl start
start:	
	mov.l	L_binary_sh_stub_bin_start,r2
	mov.l	L_binary_sh_stub_bin_end,r3
	mov.l	L_binary_sh_stub_bin_dest,r4

copy_loop:	
	mov.b	@r2+,r0
	mov.b	r0,@r4
	cmp/hs	r3,r2
	bf.s	copy_loop
	 add	#1,r4

	mov	#0,r0
	mov.l	L_STBCR,r1
	mov.b	r0,@r1
	
	mov.l	L_binary_sh_stub_bin_entry,r0
	jmp	@r0
	 nop

	.align 2
L_binary_sh_stub_bin_start:
	.long	_binary_______sh_stub_bin_start
L_binary_sh_stub_bin_end:
	.long	_binary_______sh_stub_bin_end
L_binary_sh_stub_bin_dest:
	.long	0xacff0000
L_binary_sh_stub_bin_entry:
	.long	0xacff0000
L_STBCR:
	.long	0xffc00004

	.end
