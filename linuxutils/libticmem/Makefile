
OBJS=cmem.o

#DVSDKPATH=/opt/dvsdk
#LINUXUTILSPATH=$(DVSDKPATH)/linuxutils_2_26_02_05
LINUXUTILSPATH=/home/jay/git/ti/ipc-ludev

CFLAGS=-O2 -I$(LINUXUTILSPATH)/include -fPIC -Wall
LDFLAGS=
LIBS=

CC=/usr/bin/arm-linux-gnueabihf-gcc
AR=/usr/bin/arm-linux-gnueabihf-ar
STRIP=/usr/bin/arm-linux-gnueabihf-strip

all: libticmem.so libticmem.a

libticmem.so: $(OBJS)
	$(CC) -shared -o libticmem.so $(LDFLAGS) $(OBJS) $(LIBS)
	$(STRIP) libticmem.so

libticmem.a: $(OBJS)
	$(AR) -rv libticmem.a $(OBJS)

clean:
	rm -f $(OBJS) *.c libticmem.*

%.o: $(LINUXUTILSPATH)/src/cmem/api/%.c
	$(CC) $(CFLAGS) -c $<
