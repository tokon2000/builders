
XRDP_TOP_SRC_PATH = ../../../../xrdp

OBJS = fifo.o file.o os_calls.o list.o log.o thread_calls.o list16.o \
pixman-region16.o ssl_calls.o trans.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I..

LDFLAGS = -ldl -lpthread -lssl

libcommon.so: $(OBJS)
	$(CC) --shared -o libcommon.so $(OBJS) $(LDFLAGS)

clean:
	rm -f xrdp-common.so $(OBJS)

fifo.o: $(XRDP_TOP_SRC_PATH)/common/fifo.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/fifo.c

file.o: $(XRDP_TOP_SRC_PATH)/common/file.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/file.c

list.o: $(XRDP_TOP_SRC_PATH)/common/list.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/list.c

log.o: $(XRDP_TOP_SRC_PATH)/common/log.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/log.c

os_calls.o: $(XRDP_TOP_SRC_PATH)/common/os_calls.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/os_calls.c

thread_calls.o: $(XRDP_TOP_SRC_PATH)/common/thread_calls.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/thread_calls.c

list16.o: $(XRDP_TOP_SRC_PATH)/common/list16.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/list16.c

pixman-region16.o: $(XRDP_TOP_SRC_PATH)/common/pixman-region16.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/pixman-region16.c

ssl_calls.o: $(XRDP_TOP_SRC_PATH)/common/ssl_calls.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/ssl_calls.c

trans.o: $(XRDP_TOP_SRC_PATH)/common/trans.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/common/trans.c

