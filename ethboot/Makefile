obj-y := $(patsubst %.c,%.o,$(wildcard *.c))

built-in.o: $(obj-y)
	$(LD) $(LDFLAGS) -r $^ -o $@ 

-include $(obj-y:%.o=.%.P)