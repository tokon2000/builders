
include ../Makefile.inc

OBJS = xup.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../common -lcommon

libxup.so: $(OBJS)
	$(CC) --shared -o libxup.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libxup.so $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/xup/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install libxup.so  $(XRDP_INSTALL_PATH)
