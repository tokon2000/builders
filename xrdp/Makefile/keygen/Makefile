
include ../Makefile.inc

OBJS = keygen.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../common -lcommon \
-Wl,--rpath,$(XRDP_INSTALL_PATH)/lib/xrdp

all: xrdp-keygen

xrdp-keygen: $(OBJS)
	$(CC) -o xrdp-keygen $(OBJS) $(LDFLAGS)

clean:
	rm -f xrdp-keygen $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/keygen/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install -D xrdp-keygen $(XRDP_INSTALL_PATH)/bin/xrdp-keygen
