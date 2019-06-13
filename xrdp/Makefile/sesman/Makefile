
include ../Makefile.inc

OBJS = access.o scp_v0.o session.o config.o scp_v1_mng.o sig.o env.o scp_v1.o \
verify_user_pam.o scp.o sesman.o xauth.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common \
-I$(XRDP_TOP_SRC_PATH)/sesman/libscp

LDFLAGS = -L../common -lcommon -Llibscp -lscp -lpthread -lpam \
-Wl,--rpath,$(XRDP_INSTALL_PATH)/lib/xrdp

all: libscp/libscp.so xrdp-sesman chansrv/xrdp-chansrv

libscp/libscp.so:
	$(MAKE) -C libscp

chansrv/xrdp-chansrv:
	$(MAKE) -C chansrv

clean:
	$(MAKE) -C libscp clean
	$(MAKE) -C chansrv clean
	rm -f xrdp-sesman $(OBJS)

install:
	$(MAKE) -C libscp install
	$(MAKE) -C chansrv install
	install -D xrdp-sesman $(XRDP_INSTALL_PATH)/sbin/xrdp-sesman
	install -D $(XRDP_TOP_SRC_PATH)/sesman/sesman.ini $(XRDP_INSTALL_PATH)/etc/xrdp/sesman.ini
	install -D $(XRDP_TOP_SRC_PATH)/sesman/startwm.sh $(XRDP_INSTALL_PATH)/etc/xrdp/startwm.sh
	install -D $(XRDP_TOP_SRC_PATH)/sesman/reconnectwm.sh $(XRDP_INSTALL_PATH)/etc/xrdp/reconnectwm.sh

xrdp-sesman: $(OBJS)
	$(CC) -o xrdp-sesman $(OBJS) $(LDFLAGS)

%.o: $(XRDP_TOP_SRC_PATH)/sesman/%.c
	$(CC) $(CFLAGS) -c $^
