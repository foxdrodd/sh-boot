/*
 *	$Id: kernel-boot.S,v 1.2 2003/12/12 15:34:44 lethal Exp $
 *	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>
 */
	
	.text

	.globl start
start:	
	mov.l	L_binary_zImage_bin_start,r2
	mov.l	L_binary_zImage_bin_end,r3
	mov.l	L_binary_zImage_bin_dest,r4
copy_loop1:	
	mov.b	@r2+,r0
	mov.b	r0,@r4
	cmp/hs	r3,r2
	bf.s	copy_loop1
	 add	#1,r4

	mov.l	L_binary_initrd_bin_start,r2
	mov.l	L_binary_initrd_bin_end,r3
	mov.l	L_binary_initrd_bin_dest,r4
copy_loop2:	
	mov.b	@r2+,r0
	mov.b	r0,@r4
	cmp/hs	r3,r2
	bf.s	copy_loop2
	 add	#1,r4

	mov	#0,r0
	mov.l	L_STBCR,r1
	mov.b	r0,@r1
	
	mova	bootparams,r0
	mov	r0,r2
bootparam_loop:	
	mov.l	@r2+,r0
	mov.l	@r2+,r1
	cmp/eq	#0,r0
	bt	bootparam_end
	bra	bootparam_loop
	 mov.l	r1,@r0
bootparam_end:	

	mova	commandline,r0
	mov	r0,r1
	mov.l	L_COMMANDLINE_DEST,r2
commandline_loop:
	mov.b	@r1+,r0
	mov.b	r0,@r2
	cmp/eq	#0,r0
	bf.s	commandline_loop
	 add	#1,r2
	
	mov.l	L_entry,r0
	jmp	@r0
	 nop

	.align 2
L_binary_zImage_bin_start:
	.long	_binary_zImage_bin_start
L_binary_zImage_bin_end:
	.long	_binary_zImage_bin_end
L_binary_zImage_bin_dest:
	.long	0xac710000
L_binary_initrd_bin_start:
	.long	_binary_initrd_bin_start
L_binary_initrd_bin_end:
	.long	_binary_initrd_bin_end
L_binary_initrd_bin_dest:
	.long	0xac800000
L_entry:
	.long	0x8c710000
L_STBCR:
	.long	0xffc00004
bootparams:
	.long	0x8c001000, 1		! MOUNT_ROOT_RDONLY
	.long	0x8c001004, 0		! RAMDISK_FLAGS
	.long	0x8c001008, 0x0100	! ORIG_ROOT_DEV
	.long	0x8c00100c, 1		! LOADER_TYPE
	.long	0x8c001010, 0x00800000	! INITRD_START
	.long	0x8c001014, 0x00400000	! INITRD_SIZE
	.long	0, 0	
L_COMMANDLINE_DEST:
	.long	0x8c001100
commandline:
	.ascii	"mem=16M ramdisk=8192M"
	.byte	0

	.end
