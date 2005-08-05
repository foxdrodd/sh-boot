# $Id: Makefile,v 1.44 2003/08/31 04:55:33 yoshii Exp $
#
# gdb-sh-stub/Makefile
#

TOPDIR =$(CURDIR)

include config.mk
include Rules.mak

export TOPDIR CROSS_COMPILE CC LD OBJCOPY NM CFLAGS AFLAGS LDFLAGS

LINKSCRIPT       = sh-stub.lds
LINKFLAGS	+= -T $(word 1,$(LINKSCRIPT)) -e start $(LDFLAGS)
LIBGCC           = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

common-y := entry.o main.o sh-stub.o sh-sci.o setjmp.o string.o ctype.o vprintf.o rtc.o

# machine dependent files
mdep-$(CONFIG_ARN44)            :=      init-arn44.o cs89x0.o
mdep-$(CONFIG_CQSH4)		:=	init-cqsh4.o
mdep-$(CONFIG_CQSH3)		:=	init-cqsh3.o
mdep-$(CONFIG_CAT68701)		:=	init-cat68701.o cat68701.o 
mdep-$(CONFIG_SE09)		:=	init-se09.o
mdep-$(CONFIG_SESH3)		:=	init-sesh3.o
mdep-$(CONFIG_SESHMV)		:=	init-seshmv.o
mdep-$(CONFIG_SESH4)		:=	init-sesh4.o
mdep-$(CONFIG_SESHM3)		:=	init-seshm3.o
mdep-$(CONFIG_APSH4)		:=	init-apsh4.o
mdep-$(CONFIG_HP600)		:=	init-hp600.o
mdep-$(CONFIG_DREAMCAST)	:=	init-dreamcast.o
mdep-$(CONFIG_7750OVERDRIVE)	:=	init-od7750.o
mdep-$(CONFIG_STB1OVERDRIVE)	:=	init-odstb1.o
mdep-$(CONFIG_ADX)		:=	init-adx.o

ifdef CONFIG_PCI
mdep-$(CONFIG_CPU_SUBTYPE_SH7751) +=	pci-sh7751.o
endif

mdep-$(CONFIG_IDE)      	+= 	ide.o

# ethernet controller
mdep-$(CONFIG_NE2000)		+=	ns8390.o
mdep-$(CONFIG_STNIC)		+=	ns8390.o
mdep-$(CONFIG_SMC9000)		+=	smc9000.o
mdep-$(CONFIG_PCNET32)		+=	pcnet32.o
mdep-$(CONFIG_CS89X0)		+=	cs89x0.o

ifdef CONFIG_CPU_SH2
mdep-y                          +=      exceptions-sh2.o
else
mdep-y                          +=      exceptions-sh3.o
endif

# Provided for further sub directories to be added.
subdir-$(CONFIG_ETHERNET) 	+= ethboot/

obj-y 				:= $(common-y) $(mdep-y)
all-y				:= $(obj-y) $(patsubst %/, %/built-in.o, $(subdir-y))

# get symbol address from ELF image
symbol-address = $(shell $(NM) $(2) | grep $(1) | cut -d' ' -f1)

.PHONY: all
all: sh-stub.srec

sh-stub.srec: sh-stub.elf
	$(OBJCOPY) -O srec --adjust-vma=${CONFIG_ADJUST_VMA} \
		$< $@

sh-stub.bin: sh-stub.elf
	$(OBJCOPY) -O binary $< $@

sh-stub.elf: sh-stub.exe
	$(OBJCOPY) -S -R .stack -R .bss -R .comment  \
		--change-section-lma=.data=0x$(call symbol-address,_etext,$<) \
		$< $@

sh-stub.hex: sh-stub.exe
	$(OBJCOPY) -S -R .data -R .stack -R .bss -R .comment \
		--adjust-vma=${CONFIG_ADJUST_VMA} -O ihex \
		$< $@

sh-stub.exe: $(all-y) sh-stub.lds
	$(LD) $(LINKFLAGS) $(all-y) -o $@ $(LIBGCC)
	$(NM) -n $@ > $(subst exe,map, $@)

sh-stub.lds: sh-stub.lds.S
	$(CC) -D__ASSEMBLY__ -traditional -E -C -P -I. $< > $@


.PHONY: config
config: config.h
config.h: config.mk
	@tools/mkconfigh.pl $^ > $@

.PHONY: clean
clean:
	@find . \( -name '*.[oas]' -o -name '*~' -o -name '*.exe' \
		-o -name '*.elf' -o -name '*.map' -o -name '.*.P' \) \
		-type f -print | xargs rm -f
	@$(RM) *.srec *.hex sh-stub.lds config.h

.PHONY: mrproper
mrproper: clean
	@$(RM) config.h config.mk


ifdef CONFIG_CQSH3
cqsh3: tools/cqsh3rom sh-stub.bin
	./tools/cqsh3rom sh-stub.bin sh-stub.rom

tools/cqsh3rom: tools/cqsh3.c
	gcc tools/cqsh3.c -O2 -o tools/cqsh3rom

OTHER_FILES = tools/cqsh3rom sh-stub.rom
endif

-include $(obj-y:%.o=.%.P)
