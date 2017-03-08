
XRDP_TOP_SRC_PATH = ../../../../xrdp

OBJS = xrdp.o xrdp_listen.o xrdp_process.o xrdp_region.o xrdp_wm.o \
xrdp_painter.o xrdp_bitmap.o funcs.o lang.o xrdp_cache.o xrdp_mm.o \
xrdp_font.o xrdp_encoder.o xrdp_login_wnd.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common -I$(XRDP_TOP_SRC_PATH)/libxrdp

LDFLAGS = -L../common -lcommon -L../libxrdp -lxrdp

xrdp: $(OBJS)
	$(CC) -o xrdp $(OBJS) $(LDFLAGS)

clean:
	rm -f xrdp $(OBJS)

xrdp.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp.c

xrdp_listen.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_listen.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_listen.c

xrdp_process.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_process.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_process.c

xrdp_region.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_region.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_region.c

xrdp_wm.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_wm.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_wm.c

xrdp_painter.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_painter.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_painter.c

xrdp_bitmap.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_bitmap.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_bitmap.c

funcs.o: $(XRDP_TOP_SRC_PATH)/xrdp/funcs.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/funcs.c

lang.o: $(XRDP_TOP_SRC_PATH)/xrdp/lang.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/lang.c

xrdp_cache.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_cache.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_cache.c

xrdp_mm.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_mm.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_mm.c

xrdp_font.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_font.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_font.c

xrdp_encoder.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_encoder.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_encoder.c

xrdp_login_wnd.o: $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_login_wnd.c
	$(CC) $(CFLAGS) -c $(XRDP_TOP_SRC_PATH)/xrdp/xrdp_login_wnd.c

