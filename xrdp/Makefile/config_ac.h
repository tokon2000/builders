
#if !defined _CONFIG_AC_H
#define _CONFIG_AC_H

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#define HAVE_STDINT_H 1
#define XRDP_LOG_PATH "/tmp"
#define PACKAGE_VERSION "0.9.4"

#define XRDP_SOCKET_PATH "/tmp/.xrdp"
#define XRDP_CFG_PATH "/etc/xrdp"
#define XRDP_PID_PATH "/var/run"
#define XRDP_SHARE_PATH "/usr/share"
#define XRDP_MODULE_PATH "/usr/local/lib/xrdp"

#endif
