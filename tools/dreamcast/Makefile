#! /usr/bin/make -f
#	$Id: Makefile,v 1.2 2003/12/12 15:34:44 lethal Exp $
#	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>

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
	$(OBJCOPY) -I binary -O elf32-shl $^ $@

kernel-boot.elf: kernel-boot.S zImage.o initrd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LINKFLAGS)

sh-stub-boot.elf: sh-stub-boot.S sh-stub.o
	$(CC) $(CFLAGS) -o $@ $^ $(LINKFLAGS)

sh-stub.o: ../../sh-stub.bin
	$(OBJCOPY) -I binary -O elf32-shl $^ $@

scramble: scramble.c
	gcc -o $@ $<
