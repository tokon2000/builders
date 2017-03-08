
XRDP_TOP_SRC_PATH = ../../../../xrdp

OBJS = libxrdp.o xrdp_rdp.o xrdp_sec.o xrdp_mcs.o xrdp_iso.o xrdp_orders.o \
xrdp_orders_rail.o xrdp_fastpath.o xrdp_jpeg_compress.o xrdp_channel.o \
xrdp_caps.o xrdp_bitmap_compress.o xrdp_bitmap32_compress.o xrdp_mppc_enc.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../common -lcommon

libxrdp.so: $(OBJS)
	$(CC) --shared -o libxrdp.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libxrdp.so $(OBJS)

libxrdp.o: $(XRDP_TOP_SRC_PATH)/libxrdp/libxrdp.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/libxrdp.c

xrdp_rdp.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_rdp.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_rdp.c

xrdp_sec.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_sec.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_sec.c

xrdp_mcs.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_mcs.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_mcs.c

xrdp_iso.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_iso.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_iso.c

xrdp_orders.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_orders.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_orders.c

xrdp_orders_rail.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_orders_rail.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_orders_rail.c

xrdp_fastpath.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_fastpath.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_fastpath.c

xrdp_jpeg_compress.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_jpeg_compress.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_jpeg_compress.c

xrdp_channel.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_channel.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_channel.c

xrdp_caps.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_caps.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_caps.c

xrdp_bitmap_compress.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_bitmap_compress.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_bitmap_compress.c

xrdp_bitmap32_compress.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_bitmap32_compress.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_bitmap32_compress.c

xrdp_mppc_enc.o: $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_mppc_enc.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/libxrdp/xrdp_mppc_enc.c

