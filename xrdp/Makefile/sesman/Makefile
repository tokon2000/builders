
include ../Makefile.inc

OBJS = access.o scp_v0.o session.o config.o scp_v1_mng.o sig.o env.o scp_v1.o verify_user_pam.o scp.o sesman.o xauth.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I.. -I$(XRDP_TOP_SRC_PATH)/common -I$(XRDP_TOP_SRC_PATH)/sesman/libscp

LDFLAGS = -L../common -lcommon -Llibscp -lscp -lpthread -lpam

all: libscp/libscp.so sesman

libscp/libscp.so:
	$(MAKE) -C libscp

clean:
	$(MAKE) -C libscp clean
	rm -f sesman $(OBJS)

install:
	$(MAKE) -C libscp install
	install sesman  $(XRDP_INSTALL_PATH)

sesman: $(OBJS)
	$(CC) -o sesman $(OBJS) $(LDFLAGS)

%.o: $(XRDP_TOP_SRC_PATH)/sesman/%.c
	$(CC) $(CFLAGS) -c $^
