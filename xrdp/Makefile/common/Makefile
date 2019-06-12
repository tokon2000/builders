
include ../Makefile.inc

OBJS = fifo.o file.o os_calls.o list.o log.o thread_calls.o list16.o \
pixman-region16.o ssl_calls.o trans.o base64.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I..

LDFLAGS = -ldl -lpthread -lssl

libcommon.so: $(OBJS)
	$(CC) --shared -o libcommon.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libcommon.so $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/common/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install libcommon.so  $(XRDP_INSTALL_PATH)
