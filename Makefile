# $Id: Makefile,v 1.37 2001/06/06 12:29:07 sugioka Exp $
#
# gdb-sh-stub/Makefile
#

.S.o:
	$(CC) -D__ASSEMBLY__ $(AFLAGS) -traditional -c -o $*.o $<

include config.mk

CROSS_COMPILE= sh3-linux-

CC	=$(CROSS_COMPILE)gcc -I.
LD	=$(CROSS_COMPILE)ld
OBJCOPY =$(CROSS_COMPILE)objcopy
NM	=$(CROSS_COMPILE)nm

CFLAGS = -O1 -g -Wall

ifdef CONFIG_CPU_SH3
CFLAGS		+= -m3
AFLAGS		+= -m3
endif
ifdef CONFIG_CPU_SH4
CFLAGS		+= -m4-nofpu
AFLAGS		+= -m4-nofpu
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

ifdef CONFIG_TEST4
MACHINE_DEPENDS :=	init-cqsh4.o
MEM := test4.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_CQSH4
MACHINE_DEPENDS :=	init-cqsh4.o
MEM := cqsh4.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x88002000(address for flash prog)-0xa0000000(final destination) = 0xe8002000
# 0xA8100000(address for flash prog)-0xa0000000(final destination) = 0x08100000
ADJUST_VMA=0x08100000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_TEST
MACHINE_DEPENDS :=	init-cqsh3.o
MEM := test.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_CQSH3
MACHINE_DEPENDS :=	init-cqsh3.o
MEM := cqsh3.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000

cqsh3: tools/cqsh3rom sh-stub.bin
	./tools/cqsh3rom sh-stub.bin sh-stub.rom

tools/cqsh3rom: tools/cqsh3.c
	gcc tools/cqsh3.c -O2 -o tools/cqsh3rom

OTHER_FILES = tools/cqsh3rom sh-stub.rom

endif
ifdef CONFIG_CAT68701
MACHINE_DEPENDS :=      init-cat68701.o cat68701.o
MEM := cat68701.mem
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_SH2000
MACHINE_DEPENDS :=      init-sh2000.o sh2000.o
MEM := sh2000.mem
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_TEST3
MACHINE_DEPENDS :=	init-sesh3.o
MEM := test3.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_SESH3
MACHINE_DEPENDS :=	init-sesh3.o
MEM := sesh3.mem
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

ADJUST_VMA=0x0

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_SESH4
MACHINE_DEPENDS :=	init-sesh4.o
MEM := sesh3.mem

ifdef CONFIG_ETHERNET
ifndef CONFIG_CPU_SUBTYPE_SH7751
MACHINE_DEPENDS +=	ns8390.o
endif
endif
ifdef CONFIG_CPU_SUBTYPE_SH7751
  ifdef CONFIG_PCI
MACHINE_DEPENDS +=	pci-sh7751.o
  endif
  ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	pcnet32.o
  endif
endif

#
ADJUST_VMA=0x0

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
	$(NM) -n sh-stub.exe > sh-stub.map

ifdef CONFIG_CPU_SUBTYPE_SH7751
sh-stub-ram.srec: sh-stub.srec
	$(OBJCOPY) -I srec -O srec sh-stub.srec sh-stub-ram.srec \
		--change-section-lma=.sec1=0xac000000
endif
endif
ifdef CONFIG_APSH4
MACHINE_DEPENDS :=	init-apsh4.o
MEM := apsh4.mem
# 0x00010000(address for ROM image)-0xa0010000(final destination) = 0x60000000
ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_HP600
MACHINE_DEPENDS :=	init-hp600.o
MEM := hp600.mem
#
ADJUST_VMA=0x0
endif
ifdef CONFIG_DREAMCAST
MACHINE_DEPENDS :=	init-dreamcast.o
MEM := dreamcast.mem
#
ADJUST_VMA=0x0
endif
ifdef CONFIG_7750OVERDRIVE
MACHINE_DEPENDS :=      init-od7750.o
MEM := od7750.mem
#
ADJUST_VMA=0x00000000
endif
ifdef CONFIG_STB1OVERDRIVE
MACHINE_DEPENDS :=      init-odstb1.o
MEM := odstb1.mem
#
ADJUST_VMA=0x00000000
endif

ifdef CONFIG_IDE
MACHINE_DEPENDS +=	ide.o
endif

sh-stub.elf: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		sh-stub.exe sh-stub.elf

sh-stub.hex: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} -O ihex \
		sh-stub.exe sh-stub.hex

sh-stub.bin: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment -O binary \
		sh-stub.exe sh-stub.bin

sh-stub.exe: main.o sh-stub.o entry.o ${MACHINE_DEPENDS} sh-sci.o setjmp.o string.o sh-stub.lds
	$(LD) $(LINKFLAGS) entry.o main.o sh-stub.o ${MACHINE_DEPENDS} \
		sh-sci.o setjmp.o string.o \
		-o sh-stub.exe

sh-stub.lds: sh-stub.lds.S machine/${MEM}
	$(CC) -traditional -E -C -P -I. -D MEM=\"machine/${MEM}\" sh-stub.lds.S >sh-stub.lds

clean:
	rm -rf sh-stub.exe sh-stub.bin sh-stub.elf *.o sh-stub.lds \
	       sh-stub.hex sh-stub{,-ram}.srec sh-stub.map ${OTHER_FILES}
