#! /usr/bin/make -f
#	$Id: Makefile,v 1.4 2005/08/05 02:27:46 lethal Exp $
#	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>

#
# Uncomment this to force linking in the initrd into the kernel-boot.bin
# image. This is turned off by default, as people should be using
# initramfs instead. -- PFM.
#
# INITRD_SUPPORT	:= true

TARGETS		= sh-stub-boot.bin kernel-boot.bin

CROSS_COMPILE	= sh-linux-

CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy
CFLAGS		= -ml -m4
LINKFLAGS	= -nostdlib -nostartfiles -T boot.lds

all::	$(TARGETS) scramble

clean::
	rm -f $(TARGETS) scramble
	rm -f *.elf *.o *~

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.o: %.bin
	$(OBJCOPY) -I binary -O elf32-sh-linux $^ $@

objs := zImage.o

ifdef INITRD_SUPPORT
objs += initrd.o
CFLAGS += -DHAVE_INITRD
endif

kernel-boot.elf: kernel-boot.S $(objs)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKFLAGS)

sh-stub-boot.elf: sh-stub-boot.S sh-stub.o
	$(CC) $(CFLAGS) -o $@ $^ $(LINKFLAGS)

sh-stub.o: ../../sh-stub.bin
	$(OBJCOPY) -I binary -O elf32-sh-linux $^ $@

scramble: scramble.c
	gcc -o $@ $<
