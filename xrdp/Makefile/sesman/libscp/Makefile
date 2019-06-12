
include ../../Makefile.inc

OBJS = libscp_connection.o libscp_init.o libscp_lock.o libscp_session.o libscp_tcp.o \
libscp_v0.o libscp_v1c_mng.o libscp_v1c.o libscp_v1s_mng.o libscp_v1s.o libscp_vX.o

CFLAGS = -Wall -O2 -fPIC -DHAVE_CONFIG_H -I../.. -I$(XRDP_TOP_SRC_PATH)/common

LDFLAGS = -L../../common -lcommon

libxrdp.so: $(OBJS)
	$(CC) --shared -o libscp.so $(OBJS) $(LDFLAGS)

clean:
	rm -f libscp.so $(OBJS)

%.o: $(XRDP_TOP_SRC_PATH)/sesman/libscp/%.c
	$(CC) $(CFLAGS) -c $^

install:
	install -D libscp.so $(XRDP_INSTALL_PATH)/lib/xrdp/libscp.so
