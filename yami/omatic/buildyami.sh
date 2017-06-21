#!/bin/sh

INSTALL_PATH=/opt/yami
LIBDRM_CONFIG="--disable-radeon --disable-amdgpu --disable-nouveau --disable-vmwgfx --disable-libkms"
LIBVA_CONFIG="--disable-x11 --disable-wayland"
LIBVA_INTER_DRIVER_CONFIG="--disable-x11 --disable-wayland"
LIBYAMI_CONFIG="--disable-x11"

export PKG_CONFIG_PATH=$INSTALL_PATH/lib/pkgconfig
export NOCONFIGURE=1

rm -r $INSTALL_PATH/*

rm -f libdrm-2.4.74.tar.gz
#wget https://dri.freedesktop.org/libdrm/libdrm-2.4.74.tar.gz
wget http://server1.xrdp.org/yami/libdrm-2.4.74.tar.gz
if test $? -ne 0
then
  echo "error downloading libdrm-2.4.74.tar.gz"
  exit 1
fi

rm -f libva-1.7.3.tar.bz2
rm -f libva-1.7.3.tar
#wget https://www.freedesktop.org/software/vaapi/releases/libva/libva-1.7.3.tar.bz2
wget http://server1.xrdp.org/yami/libva-1.7.3.tar.bz2
if test $? -ne 0
then
  echo "error downloading libva-1.7.3.tar.bz2"
  exit 1
fi

rm -f libva-intel-driver-1.7.3.tar.bz2
rm -f libva-intel-driver-1.7.3.tar
#wget https://www.freedesktop.org/software/vaapi/releases/libva-intel-driver/libva-intel-driver-1.7.3.tar.bz2
wget http://server1.xrdp.org/yami/libva-intel-driver-1.7.3.tar.bz2
if test $? -ne 0
then
  echo "error downloading libva-intel-driver-1.7.3.tar.bz2"
  exit 1
fi

rm -fr libdrm-2.4.74
tar -zxf libdrm-2.4.74.tar.gz
cd libdrm-2.4.74
./configure --prefix=$INSTALL_PATH $LIBDRM_CONFIG
if test $? -ne 0
then
  echo "error configure libdrm"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make libdrm"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install libdrm"
  exit 1
fi
cd ..

rm -fr libva-1.7.3
bunzip2 libva-1.7.3.tar.bz2
tar -xf libva-1.7.3.tar
cd libva-1.7.3
./configure --prefix=$INSTALL_PATH $LIBVA_CONFIG
if test $? -ne 0
then
  echo "error configure libva"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make libva"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install libva"
  exit 1
fi
cd ..

rm -rf libva-intel-driver-1.7.3
bunzip2 libva-intel-driver-1.7.3.tar.bz2
tar -xf libva-intel-driver-1.7.3.tar
cd libva-intel-driver-1.7.3
./configure --prefix=$INSTALL_PATH $LIBVA_INTER_DRIVER_CONFIG
if test $? -ne 0
then
  echo "error configure libva-intel-driver"
  exit 1
fi
make
if test $? -ne 0
then
  echo "error make libva-intel-driver"
  exit 1
fi
make install-strip
if test $? -ne 0
then
  echo "error make install libva-intel-driver"
  exit 1
fi
cd ..

rm -rf libyami
git clone https://github.com/01org/libyami.git
#git clone https://github.com/jsorg71/libyami.git
#git clone https://github.com/lizhong1008/libyami.git
cd libyami
#git checkout infinte_gop
#git checkout apache
#git checkout fa3865a3406f9f21b729d5b6d46536a7e70eb391
git checkout 1.1.0
./autogen.sh
./configure --prefix=$INSTALL_PATH $LIBYAMI_CONFIG
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

