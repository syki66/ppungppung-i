CC  := /opt/gnueabi/opt/ext-toolchain/bin/arm-linux-gnueabihf-gcc


.SUFFIXES : .c .o 
CFLAGS = -O2 -g -static 

OBJS1 = fnd_test.o 
OBJS2 = clcd_test.o 
OBJS3 = dot_test.o 
OBJS4 = tactsw_test.o 
OBJS5 = led_test.o 
OBJS6 = st_motor_test.o 
OBJS7 = dipsw_test.o 
SRCS = $(OBJS1:.o=.c) \
	$(OBJS2:.o=.c) \
	$(OBJS3:.o=.c) \
	$(OBJS4:.o=.c) \
	$(OBJS5:.o=.c) \
	$(OBJS6:.o=.c) \
	$(OBJS7:.o=.c) \

all : fnd_test clcd_test dot_test tactsw_test led_test st_motor_test dipsw_test

fnd_test : $(OBJS1)
	$(CC) $(CFLAGS) -o fnd_test $(OBJS1)

clcd_test : $(OBJS2)
	$(CC) $(CFLAGS) -o clcd_test $(OBJS2)

dot_test : $(OBJS3)
	$(CC) $(CFLAGS) -o dot_test $(OBJS3)

key_test : $(OBJS4)
	$(CC) $(CFLAGS) -o tactsw_test $(OBJS4)

led_test : $(OBJS5)
	$(CC) $(CFLAGS) -o led_test $(OBJS5)

st_motor_test : $(OBJS6)
	$(CC) $(CFLAGS) -o st_motor_test $(OBJS6)

dipsw_test : $(OBJS7)
	$(CC) $(CFLAGS) -o dipsw_test $(OBJS7)

.c.o : 
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -f *.o *_test 
