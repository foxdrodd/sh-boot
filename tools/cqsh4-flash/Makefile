CROSS_COMPILE= sh3-linux-

CC	=$(CROSS_COMPILE)gcc -I.
LD	=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

flash:	flash.o
	$(LD) -T shelf.x -EL -e do_flash flash.o -o flash

flash.o: flash.c
	$(CC) -O2 -g -ml -m4-nofpu -pipe   -c flash.c -o flash.o

flash-eb:	flash-eb.o
	$(LD) -T shelf.x -EB -e do_flash flash-eb.o -o flash-eb

flash-eb.o: flash.c
	$(CC) -O2 -g -DSWAP -mb -m4-nofpu -pipe   -c flash.c -o flash-eb.o

clean:
	rm -f *.o flash flash-eb
