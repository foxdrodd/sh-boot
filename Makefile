# $Id: Makefile,v 1.44 2003/08/31 04:55:33 yoshii Exp $
#
# gdb-sh-stub/Makefile
#

.S.o:
	$(CC) -D__ASSEMBLY__ $(AFLAGS) -traditional -c -o $*.o $<

include config.mk

CROSS_COMPILE= sh-linux-

CC	=$(CROSS_COMPILE)gcc -I.
LD	=$(CROSS_COMPILE)ld
OBJCOPY =$(CROSS_COMPILE)objcopy
NM	=$(CROSS_COMPILE)nm

CFLAGS = -O2 -g -Wall

# derived from $(TOPDIR)/Makefile
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

cflags-y				:= -mb
cflags-$(CONFIG_LITTLE_ENDIAN)	        := -ml

cflags-$(CONFIG_CPU_SH3)		+= -m3
cflags-$(CONFIG_CPU_SH4)		+= -m4 \
	$(call cc-option,-mno-implicit-fp,-m4-nofpu)

CFLAGS		+= -pipe $(cflags-y)
AFLAGS		+= $(cflags-y)

ifdef CONFIG_LITTLE_ENDIAN
LDFLAGS		:= -EL
else
LDFLAGS		:= -EB
endif

LINKSCRIPT       = sh-stub.lds
LINKFLAGS	+= -T $(word 1,$(LINKSCRIPT)) -e start $(LDFLAGS)
LIBGCC           = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

ifdef CONFIG_TEST4
MACHINE_DEPENDS :=	init-cqsh4.o
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_CQSH4
MACHINE_DEPENDS :=	init-cqsh4.o
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
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif
ifdef CONFIG_CQSH3
MACHINE_DEPENDS :=	init-cqsh3.o
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
MACHINE_DEPENDS :=      init-cat68701.o cat68701.o cs89x0.o 
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000
endif

ifdef CONFIG_TEST3
MACHINE_DEPENDS :=	init-sesh3.o
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_SE09
MACHINE_DEPENDS :=	init-se09.o
#
ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ns8390.o
endif

ADJUST_VMA=0x0

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_SESH3
MACHINE_DEPENDS :=	init-sesh3.o
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

ifdef CONFIG_CPU_SUBTYPE_SH7751
sh-stub-ram.srec: sh-stub.srec
	$(OBJCOPY) -I srec -O srec sh-stub.srec sh-stub-ram.srec \
		--change-section-lma=.sec1=0xac000000
endif
endif
ifdef CONFIG_APSH4
MACHINE_DEPENDS :=	init-apsh4.o
# 0x00010000(address for ROM image)-0xa0010000(final destination) = 0x60000000
ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${ADJUST_VMA} \
		-O srec sh-stub.exe sh-stub.srec
endif
ifdef CONFIG_HP600
MACHINE_DEPENDS :=	init-hp600.o
#
ADJUST_VMA=0x0
endif
ifdef CONFIG_DREAMCAST
MACHINE_DEPENDS :=	init-dreamcast.o
#
ADJUST_VMA=0x0
endif
ifdef CONFIG_7750OVERDRIVE
MACHINE_DEPENDS :=      init-od7750.o
#
ADJUST_VMA=0x00000000
endif
ifdef CONFIG_STB1OVERDRIVE
MACHINE_DEPENDS :=      init-odstb1.o
#
ADJUST_VMA=0x00000000
endif
ifdef CONFIG_ADX
MACHINE_DEPENDS :=	init-adx.o
# 0x00000000(address for ROM image)-0xa0000000(final destination) = 0x60000000
ADJUST_VMA=0x60000000

sh-stub.srec: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		-O srec sh-stub.exe sh-stub.srec
endif

ifdef CONFIG_IDE
MACHINE_DEPENDS +=	ide.o
endif

ifdef CONFIG_ETHERNET
MACHINE_DEPENDS +=	ethboot/etherboot.o
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

sh-stub.exe: main.o sh-stub.o entry.o ${MACHINE_DEPENDS} sh-sci.o setjmp.o \
             string.o ctype.o vprintf.o sh-stub.lds
	$(LD) $(LINKFLAGS) entry.o main.o sh-stub.o ${MACHINE_DEPENDS} \
		sh-sci.o setjmp.o string.o ctype.o vprintf.o \
		-o sh-stub.exe $(LIBGCC)
	$(NM) -n sh-stub.exe > sh-stub.map

sh-stub.lds: sh-stub.lds.S
	$(CC) -D__ASSEMBLY__ -traditional -E -C -P -I. sh-stub.lds.S >sh-stub.lds

ethboot/etherboot.o: ethboot/ethboot.c ethboot/bootp.c ethboot/nfs.c ethboot/net.c ethboot/string.c
	make -C ethboot etherboot.o

clean:
	rm -rf sh-stub.exe sh-stub.bin sh-stub.elf *.o sh-stub.lds \
	       sh-stub.hex sh-stub{,-ram}.srec sh-stub.map ${OTHER_FILES}
	rm -f ethboot.lnk
	make -C ethboot clean

cs89x0.o:     config.h string.h defs.h cs89x0.h io.h
ide.o:        config.h defs.h io.h
main.o:       config.h defs.h string.h io.h
ns8390.o:     config.h string.h defs.h stnic-se.h ne.h
pci-sh7751.o: config.h defs.h io.h
pcnet32.o:    config.h string.h defs.h io.h
sh-sci.o:     config.h io.h
sh-stub.o:    config.h defs.h string.h setjmp.h
sh2000.o:     io.h
