include ../config.mk

CFLAGS		+= -I.. -DROM

OBJS = ethboot.o bootp.o nfs.o net.o string.o

all:	etherboot.o

etherboot.o: $(OBJS)
	$(LD) -r $(OBJS) -o etherboot.o

string.o: string.c defs.h
ethboot.o: defs.h osdep.h etherboot.h
bootp.o: defs.h osdep.h etherboot.h
nfs.o: defs.h osdep.h etherboot.h
net.o: defs.h osdep.h etherboot.h

clean:
	rm -f *.o core *~
