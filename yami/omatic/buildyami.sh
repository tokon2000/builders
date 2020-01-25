#!/bin/sh

INSTALL_PATH=/opt/yami
LIBDRM_CONFIG="--disable-radeon --disable-amdgpu --disable-nouveau --disable-vmwgfx --disable-libkms"
LIBVA_CONFIG="--disable-x11 --disable-wayland"
LIBVAUTILS_CONFIG="--disable-x11 --disable-wayland"
LIBVA_INTER_DRIVER_CONFIG="--disable-x11 --disable-wayland"
LIBYAMI_CONFIG="--disable-jpegdec --disable-vp8dec --disable-h265dec --enable-capi --disable-x11 --enable-mpeg2dec"
SHOW_HELP=0
ENABLE_X11=0
GOT_PARAM=0

LIBDRM_SRC_NAME="libdrm-2.4.100"
LIBVA_SRC_NAME="libva-2.5.0"
LIBVAUTILS_SRC_NAME="libva-utils-2.5.0"
LIBVA_INTER_DRIVER_SRC_NAME="intel-vaapi-driver-2.3.0"

LIBYAMI_INF_CONFIG=

for i in "$@"
do
case $i in
    --prefix=*)
    INSTALL_PATH="${i#*=}"
    GOT_PARAM=1
    shift # past argument=value
    ;;
    --enable-x11)
    ENABLE_X11=1
    GOT_PARAM=1
    shift # past argument=value
    ;;
    --disable-x11)
    ENABLE_X11=0
    shift # past argument=value
    ;;
    *)
          # unknown option
    SHOW_HELP=1
    ;;
esac
done

if test $GOT_PARAM -eq 0
then
    SHOW_HELP=1
fi

if test $SHOW_HELP -ne 0
then
    echo "./buildyami.sh [--prefix=/opt/yami] [--enable-x11 | --disable-x11]"
    exit 0
fi

if test $ENABLE_X11 -ne 0
then
    LIBVA_CONFIG="--enable-x11 --disable-wayland"
    LIBVAUTILS_CONFIG="--enable-x11 --disable-wayland"
    LIBVA_INTER_DRIVER_CONFIG="--enable-x11 --disable-wayland"
    LIBYAMI_CONFIG="--disable-jpegdec --disable-vp8dec --disable-h265dec --enable-capi --enable-x11 --enable-mpeg2dec"
    LIBYAMI_INF_CONFIG="--enable-x11"
fi

echo "INSTALL_PATH              = $INSTALL_PATH"
echo "LIBDRM_CONFIG             = $LIBDRM_CONFIG"
echo "LIBVA_CONFIG              = $LIBVA_CONFIG"
echo "LIBVAUTILS_CONFIG         = $LIBVAUTILS_CONFIG"
echo "LIBVA_INTER_DRIVER_CONFIG = $LIBVA_INTER_DRIVER_CONFIG"
echo "LIBYAMI_CONFIG            = $LIBYAMI_CONFIG"

export PKG_CONFIG_PATH=$INSTALL_PATH/lib/pkgconfig
export NOCONFIGURE=1

rm -r $INSTALL_PATH/*

rm -f $LIBDRM_SRC_NAME.tar.gz
#wget https://dri.freedesktop.org/libdrm/$LIBDRM_SRC_NAME.tar.gz
wget http://server1.xrdp.org/yami/$LIBDRM_SRC_NAME.tar.gz
if test $? -ne 0
then
  echo "error downloading $LIBDRM_SRC_NAME.tar.gz"
  exit 1
fi

rm -f $LIBVA_SRC_NAME.tar.bz2
rm -f $LIBVA_SRC_NAME.tar
#wget https://www.freedesktop.org/software/vaapi/releases/libva/$LIBVA_SRC_NAME.tar.bz2
#wget https://github.com/01org/libva/releases/download/2.3.0/$LIBVA_SRC_NAME.tar.bz2
wget http://server1.xrdp.org/yami/$LIBVA_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVA_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $LIBVAUTILS_SRC_NAME.tar.bz2
rm -f $LIBVAUTILS_SRC_NAME.tar
#wget https://www.freedesktop.org/software/vaapi/releases/libva/$LIBVAUTILS_SRC_NAME.tar.bz2
#wget https://github.com/intel/libva-utils/releases/download/2.2.0/$LIBVAUTILS_SRC_NAME.tar.bz2
wget http://server1.xrdp.org/yami/$LIBVAUTILS_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVAUTILS_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2
rm -f $LIBVA_INTER_DRIVER_SRC_NAME.tar
#wget https://www.freedesktop.org/software/vaapi/releases/libva-intel-driver/$LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2
#wget https://github.com/intel/intel-vaapi-driver/releases/download/2.3.0/$LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2
wget http://server1.xrdp.org/yami/$LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2"
  exit 1
fi

rm -fr $LIBDRM_SRC_NAME
tar -zxf $LIBDRM_SRC_NAME.tar.gz
cd $LIBDRM_SRC_NAME
./configure --prefix=$INSTALL_PATH $LIBDRM_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBDRM_SRC_NAME"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make $LIBDRM_SRC_NAME"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install $LIBDRM_SRC_NAME"
  exit 1
fi
cd ..

rm -fr $LIBVA_SRC_NAME
bunzip2 -k $LIBVA_SRC_NAME.tar.bz2
tar -xf $LIBVA_SRC_NAME.tar
cd $LIBVA_SRC_NAME
./configure --prefix=$INSTALL_PATH $LIBVA_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBVA_SRC_NAME"
  exit 1
fi
# this will get rid of libva info logging
echo "" >> config.h
echo "#define va_log_info(buffer)" >> config.h
echo "" >> config.h
make
if test $? -ne 0
then
  echo "error make $LIBVA_SRC_NAME"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install $LIBVA_SRC_NAME"
  exit 1
fi
cd ..

rm -fr $LIBVAUTILS_SRC_NAME
bunzip2 -k $LIBVAUTILS_SRC_NAME.tar.bz2
tar -xf $LIBVAUTILS_SRC_NAME.tar
cd $LIBVAUTILS_SRC_NAME
./configure --prefix=$INSTALL_PATH $LIBVAUTILS_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBVAUTILS_SRC_NAME"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make $LIBVAUTILS_SRC_NAME"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install $LIBVAUTILS_SRC_NAME"
  exit 1
fi
cd ..

rm -rf $LIBVA_INTER_DRIVER_SRC_NAME
bunzip2 -k $LIBVA_INTER_DRIVER_SRC_NAME.tar.bz2
tar -xf $LIBVA_INTER_DRIVER_SRC_NAME.tar
cd $LIBVA_INTER_DRIVER_SRC_NAME
echo "patching $LIBVA_INTER_DRIVER_SRC_NAME"
patch -p1 < ../0002-RGB-YUV-fix.patch
if test $? -ne 0
then
  echo "patching $LIBVA_INTER_DRIVER_SRC_NAME failed"
  exit 1
fi
./configure --prefix=$INSTALL_PATH $LIBVA_INTER_DRIVER_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBVA_INTER_DRIVER_SRC_NAME"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make $LIBVA_INTER_DRIVER_SRC_NAME"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install $LIBVA_INTER_DRIVER_SRC_NAME"
  exit 1
fi
cd ..

rm -rf libyami
git clone https://github.com/intel/libyami.git
#git clone https://github.com/01org/libyami.git
#git clone https://github.com/xuguangxin/libyami.git
#git clone https://github.com/jsorg71/libyami.git
#git clone https://github.com/lizhong1008/libyami.git
cd libyami
#git checkout infinte_gop
#git checkout apache
#git checkout fa3865a3406f9f21b729d5b6d46536a7e70eb391
#git checkout 1.1.0
#git checkout 1.2.0
#git checkout 1.3.0
git checkout 1.3.2
#git checkout libyami-0.3.1
#git checkout fix_low_latency
./autogen.sh
CFLAGS="-O2 -Wall" CXXFLAGS="-O2 -Wall" ./configure --prefix=$INSTALL_PATH $LIBYAMI_CONFIG
if test $? -ne 0
then
  echo "error configure libyami"
  exit 1
fi
make clean
make
if test $? -ne 0
then
  echo "error make libyami"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install libyami"
  exit 1
fi
cd ..

cd yami_inf
./bootstrap
if test $? -ne 0
then
  echo "error bootstrap yami_inf"
  exit 1
fi
./configure --prefix=$INSTALL_PATH $LIBYAMI_INF_CONFIG
if test $? -ne 0
then
  echo "error configure yami_inf"
  exit 1
fi
make clean
make
if test $? -ne 0
then
  echo "error make yami_inf"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install yami_inf"
  exit 1
fi
cd ..

