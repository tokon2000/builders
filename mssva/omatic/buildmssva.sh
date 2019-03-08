#!/bin/sh

INSTALL_PATH=/opt/mssva
LIBDRM_CONFIG="--disable-radeon --disable-amdgpu --disable-nouveau --disable-vmwgfx --disable-libkms"
LIBVA_CONFIG="--disable-x11 --disable-wayland"
LIBVAUTILS_CONFIG="--disable-x11 --disable-wayland"
SHOW_HELP=0
ENABLE_X11=0
GOT_PARAM=0

# MediaServerStudioEssentials2018_16.9_00183
LIBDRM_SRC_NAME="libdrm-2.4.74"
LIBVA_SRC_NAME="libva-2.1.1.pre1"
LIBVAUTILS_SRC_NAME="libva-utils-2.0.0"

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
    echo "./buildmssva.sh [--prefix=/opt/mssva] [--enable-x11 | --disable-x11]"
    exit 0
fi

if test $ENABLE_X11 -ne 0
then
    LIBVA_CONFIG="--enable-x11 --disable-wayland"
fi

echo "INSTALL_PATH              = $INSTALL_PATH"
echo "LIBDRM_CONFIG             = $LIBDRM_CONFIG"
echo "LIBVA_CONFIG              = $LIBVA_CONFIG"

export PKG_CONFIG_PATH=$INSTALL_PATH/lib/pkgconfig
export NOCONFIGURE=1

rm -r $INSTALL_PATH/*

rm -f $LIBDRM_SRC_NAME.tar.bz2
rm -f $LIBDRM_SRC_NAME.tar
wget http://server1.xrdp.org/yami/$LIBDRM_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBDRM_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $LIBVA_SRC_NAME.tar.bz2
rm -f $LIBVA_SRC_NAME.tar
wget http://server1.xrdp.org/yami/$LIBVA_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVA_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $LIBVAUTILS_SRC_NAME.tar.bz2
rm -f $LIBVAUTILS_SRC_NAME.tar
wget http://server1.xrdp.org/yami/$LIBVAUTILS_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVAUTILS_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f mss-2018-R2-min.tar.gz
wget http://server1.xrdp.org/yami/mss-2018-R2-min.tar.gz
if test $? -ne 0
then
  echo "error downloading mss-2018-R2-min.tar.gz"
  exit 1
fi

echo "rm -fr $LIBDRM_SRC_NAME"
rm -fr $LIBDRM_SRC_NAME
echo "bunzip2 -k $LIBDRM_SRC_NAME.tar.bz2"
bunzip2 -k $LIBDRM_SRC_NAME.tar.bz2
echo "tar -zxf $LIBDRM_SRC_NAME.tar"
tar -xf $LIBDRM_SRC_NAME.tar
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
./autogen.sh
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

cd MSS2018R2/mediasdk/opensource/mfx_dispatch
cmake . -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH
make clean
make
make install

cd ../../../..
cd mssva_inf
./bootstrap
./configure --prefix=$INSTALL_PATH
make
make install-strip
cd ..

echo "extracting mss-2018-R2-min.tar.gz"
LCD=$PWD
cd $INSTALL_PATH/lib
tar -xf $LCD/mss-2018-R2-min.tar.gz
cd $LCD
