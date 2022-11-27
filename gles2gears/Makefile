
CC=/usr/bin/arm-linux-gnueabihf-gcc

OBJS=gles2gears.o

CFLAGS=-O2 -Wall -Wextra

LDFLAGS=

LIBS=-lepoxy -lm

gles2gears: $(OBJS)
	$(CC) -o gles2gears $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	rm -f gles2gears $(OBJS)
