
include ../Makefile.inc

OBJS = libxrdp.o xrdp_rdp.o xrdp_sec.o xrdp_mcs.o xrdp_iso.o xrdp_orders.o \
xrdp_orders_rail.o xrdp_fastpath.o xrdp_jpeg_compress.o xrdp_channel.o \
xrdp_caps.o xrdp_bitmap_compress.o xrdp_bitmap32_compress.o xrdp_mppc_enc.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../common -lcommon

libxrdp.so: $(OBJS)
	$(CC) --shared -o libxrdp.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libxrdp.so $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/libxrdp/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install -D libxrdp.so $(XRDP_INSTALL_PATH)/lib/xrdp/libxrdp.so
