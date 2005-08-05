CROSS_COMPILE= sh-linux-

CC	=$(CROSS_COMPILE)gcc
CPP	=$(CROSS_COMPILE)cpp
LD	=$(CROSS_COMPILE)ld
OBJCOPY =$(CROSS_COMPILE)objcopy
NM	=$(CROSS_COMPILE)nm

CFLAGS = -I$(TOPDIR) -Os -Wall -fno-hosted -fomit-frame-pointer -pipe 

cflags-y				:= -mb
cflags-$(CONFIG_LITTLE_ENDIAN)	        := -ml


# derived from $(TOPDIR)/Makefile
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
             > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

cflags-$(CONFIG_CPU_SH2)		+= -m2 -Wa,-isa=sh2
cflags-$(CONFIG_CPU_SH3)		+= -m3
cflags-$(CONFIG_CPU_SH4)		+= -m4 \
	$(call cc-option,-mno-implicit-fp,-m4-nofpu)

CFLAGS		+= $(cflags-y)
AFLAGS		+= -I$(TOPDIR) -D__ASSEMBLY__ -traditional $(cflags-y)

ifeq ($(CONFIG_LITTLE_ENDIAN),y)
LDFLAGS		:= -EL
else
LDFLAGS		:= -EB
endif

define make-dependency
cp $*.d .$*.P
sed -e 's/#.*//'  -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> .$*.P; \
rm -f $*.d
endef

%.o : %.c
	$(COMPILE.c) -MD -o $@ $<
	@$(make-dependency)

%.o: %.S
	$(CC) $(AFLAGS) -MD -c -o $@ $<
	@$(make-dependency)

%/built-in.o:
	$(MAKE) -C $(@D)

