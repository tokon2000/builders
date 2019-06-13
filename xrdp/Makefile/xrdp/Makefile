
include ../Makefile.inc

OBJS = xrdp.o xrdp_listen.o xrdp_process.o xrdp_region.o xrdp_wm.o \
xrdp_painter.o xrdp_bitmap.o funcs.o lang.o xrdp_cache.o xrdp_mm.o \
xrdp_font.o xrdp_encoder.o xrdp_login_wnd.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common \
-I$(XRDP_TOP_SRC_PATH)/libxrdp

LDFLAGS = -L../common -lcommon -L../libxrdp -lxrdp \
-Wl,--rpath,$(XRDP_INSTALL_PATH)/lib/xrdp

all: xrdp

xrdp: $(OBJS)
	$(CC) -o xrdp $(OBJS) $(LDFLAGS)

clean:
	rm -f xrdp $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/xrdp/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install -D xrdp $(XRDP_INSTALL_PATH)/sbin/xrdp
	install -D $(XRDP_TOP_SRC_PATH)/xrdp/xrdp.ini $(XRDP_INSTALL_PATH)/etc/xrdp/xrdp.ini
