
include ../../Makefile.inc

OBJS = chansrv_common.o irp.o chansrv_fuse.o rail.o chansrv.o smartcard.o \
clipboard_file.o smartcard_pcsc.o clipboard.o sound.o devredir.o xcommon.o \
fifo.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I../.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../../common -lcommon -lpthread -lX11 -lXfixes -lXrandr \
-Wl,--rpath,$(XRDP_INSTALL_PATH)/lib/xrdp

all: xrdp-chansrv

xrdp-chansrv: $(OBJS)
	$(CC) -o xrdp-chansrv $(OBJS) $(LDFLAGS)

clean:
	rm -f xrdp-chansrv $(OBJS)

install:
	install -D xrdp-chansrv $(XRDP_INSTALL_PATH)/sbin/xrdp-chansrv

%.o: $(XRDP_TOP_SRC_PATH)/sesman/chansrv/%.c
	$(CC) $(CFLAGS) -c $^
