
include ../Makefile.inc

OBJS = vnc.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../common -lcommon

libxup.so: $(OBJS)
	$(CC) --shared -o libvnc.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libvnc.so $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/vnc/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install -D libvnc.so $(XRDP_INSTALL_PATH)/lib/xrdp/libvnc.so
