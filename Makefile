# $Id: Makefile,v 1.19 2000/06/21 13:12:53 gniibe Exp $
#
# gdb-sh-stub/Makefile
#

.S.o:
	$(CC) -D__ASSEMBLY__ $(AFLAGS) -traditional -c -o $*.o $<

include config.mk

CROSS_COMPILE= sh-linux-gnu-

CC	=$(CROSS_COMPILE)gcc -I.
LD	=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

CFLAGS = -O2 -g

ifdef CONFIG_CPU_SH3
CFLAGS		+= -m3
AFLAGS		+= -m3
endif
ifdef CONFIG_CPU_SH4
CFLAGS		+= -m4
AFLAGS		+= -m4
endif

ifdef CONFIG_LITTLE_ENDIAN
CFLAGS		+= -ml
AFLAGS		+= -ml
LINKFLAGS	+= -EL
else
CFLAGS		+= -mb
AFLAGS		+= -mb
LINKFLAGS	+= -EB
endif

CFLAGS		+= -pipe
LINKSCRIPT    = sh-stub.lds
LINKFLAGS	+= -T $(word 1,$(LINKSCRIPT)) -e start

ifdef CONFIG_CQSH4
MACHINE_DEPENDS :=	init-cqsh4.o
# 0x88002000(address for flash prog)-0xa0000000(final destination) = 0xe8002000
# 0xA8100000(address for flash prog)-0xa0000000(final destination) = 0x08100000
ADJUST_VMA=0x08100000

sh-stub.srec: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		-O srec sh-stub sh-stub.srec
endif
ifdef CONFIG_TEST
MACHINE_DEPENDS :=	init-cqsh3.o
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_CQSH3
MACHINE_DEPENDS :=	init-cqsh3.o
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_SESH3
MACHINE_DEPENDS :=	init-sesh3.o
#
ADJUST_VMA=0x0

sh-stub.srec: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub sh-stub.srec
endif
ifdef CONFIG_SESH4
MACHINE_DEPENDS :=	init-sesh4.o
#
ADJUST_VMA=0x0

sh-stub.srec: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub sh-stub.srec
endif
ifdef CONFIG_APSH4
MACHINE_DEPENDS :=	init-apsh4.o
# 0x00010000(address for ROM image)-0xa0010000(final destination) = 0x60000000
ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		-O srec sh-stub sh-stub.srec
endif
ifdef CONFIG_HP600
MACHINE_DEPENDS :=	init-hp600.o
#
ADJUST_VMA=0x0
endif

sh-stub.elf: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		sh-stub sh-stub.elf

sh-stub.hex: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} -O ihex \
		sh-stub sh-stub.hex

sh-stub.bin: sh-stub
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment -O binary \
		sh-stub sh-stub.bin

sh-stub: sh-stub.o entry.o ${MACHINE_DEPENDS} sh-sci.o setjmp.o string.o sh-stub.lds
	$(LD) $(LINKFLAGS) entry.o sh-stub.o ${MACHINE_DEPENDS} sh-sci.o \
		setjmp.o string.o \
		-o sh-stub

sh-stub.lds: sh-stub.lds.S
	$(CC) -E -C -P -I. sh-stub.lds.S >sh-stub.lds

clean:
	rm -rf sh-stub sh-stub.bin sh-stub.elf *.o sh-stub.lds sh-stub.srec
