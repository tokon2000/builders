#!/usr/bin/env bash

sudo -v

# Packages necessary for building the Intel drivers as well as for
# YAMI to initialize properly at runtime.
sudo apt-get -y install autoconf libtool libdrm-dev xorg xorg-dev \
  openbox libx11-dev libgl1-mesa-glx libegl1-mesa libegl1-mesa-dev \
  libgl1-mesa-dev meson doxygen cmake

INSTALL_PATH=/opt/yami

SHOW_HELP=0
ENABLE_X11=0
GOT_PARAM=0

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

LIBRARY_INSTALLATION_DIR=$INSTALL_PATH/lib

LIBDRM_CONFIG="-Dlibdir=$LIBRARY_INSTALLATION_DIR -Dradeon=disabled -Damdgpu=disabled -Dnouveau=disabled -Dvmwgfx=disabled"
LIBVA_CONFIG="-Ddriverdir=$LIBRARY_INSTALLATION_DIR -Dlibdir=$LIBRARY_INSTALLATION_DIR -Dwith_x11=yes -Dwith_wayland=no"
LIBVAUTILS_CONFIG="--libdir=$LIBRARY_INSTALLATION_DIR --disable-x11 --disable-wayland"
INTEL_MEDIA_DRIVER_CONFIG="-DLIBVA_DRIVERS_PATH=$LIBRARY_INSTALLATION_DIR/dri -DCMAKE_INSTALL_LIBDIR=$LIBRARY_INSTALLATION_DIR -DCMAKE_PREFIX_PATH=$INSTALL_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH"
LIBVA_INTEL_DRIVER_CONFIG="-Ddriverdir=$LIBRARY_INSTALLATION_DIR/dri -Dlibdir=$LIBRARY_INSTALLATION_DIR -Dwith_x11=no -Dwith_wayland=no"
INTEL_GMMLIB_CONFIG="-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH -DCMAKE_INSTALL_LIBDIR=$LIBRARY_INSTALLATION_DIR"
LIBYAMI_CONFIG="--disable-jpegdec --disable-vp8dec --disable-h265dec --enable-capi --disable-x11 --enable-mpeg2dec"

LIBDRM_SRC_NAME="drm-libdrm-2.4.120"
LIBVA_SRC_NAME="libva-2.21.0"
LIBVAUTILS_SRC_NAME="libva-utils-2.21.0"
INTEL_MEDIA_DRIVER_SRC_NAME="intel-media-24.1.5"
LIBVA_INTEL_DRIVER_SRC_NAME="intel-vaapi-driver-2.4.1"
INTEL_GMMLIB_SRC_NAME="intel-gmmlib-22.3.19"
LIBYAMI_INF_CONFIG=

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
    LIBVA_INTEL_DRIVER_CONFIG="--enable-x11 --disable-wayland"
    LIBYAMI_CONFIG="--disable-jpegdec --disable-vp8dec --disable-h265dec --enable-capi --enable-x11 --enable-mpeg2dec"
    LIBYAMI_INF_CONFIG="--enable-x11"
fi

echo "INSTALL_PATH                = $INSTALL_PATH"
echo "LIBDRM_CONFIG               = $LIBDRM_CONFIG"
echo "LIBVA_CONFIG                = $LIBVA_CONFIG"
echo "LIBVAUTILS_CONFIG           = $LIBVAUTILS_CONFIG"
echo "INTEL_MEDIA_DRIVER_SRC_NAME = $INTEL_MEDIA_DRIVER_SRC_NAME"
echo "LIBVA_INTEL_DRIVER_CONFIG   = $LIBVA_INTEL_DRIVER_CONFIG"
echo "INTEL_GMMLIB_SRC_NAME       = $INTEL_GMMLIB_SRC_NAME"
echo "LIBYAMI_CONFIG              = $LIBYAMI_CONFIG"

export PKG_CONFIG_PATH="$LIBRARY_INSTALLATION_DIR/pkgconfig"
export NOCONFIGURE=1

rm -r $INSTALL_PATH/*

rm -f $LIBDRM_SRC_NAME.tar.gz
wget https://gitlab.freedesktop.org/mesa/drm/-/archive/libdrm-2.4.120/$LIBDRM_SRC_NAME.tar.gz
if test $? -ne 0
then
  echo "error downloading $LIBDRM_SRC_NAME.tar.gz"
  exit 1
fi

rm -f $LIBVA_SRC_NAME.tar.bz2
rm -f $LIBVA_SRC_NAME.tar
wget https://github.com/intel/libva/releases/download/2.21.0/$LIBVA_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVA_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $LIBVAUTILS_SRC_NAME.tar.bz2
rm -f $LIBVAUTILS_SRC_NAME.tar
wget https://github.com/intel/libva-utils/releases/download/2.21.0/$LIBVAUTILS_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVAUTILS_SRC_NAME.tar.bz2"
  exit 1
fi

# rm -f $INTEL_MEDIA_DRIVER_SRC_NAME.tar.gz
# wget https://github.com/intel/media-driver/archive/refs/tags/$INTEL_MEDIA_DRIVER_SRC_NAME.tar.gz
# if test $? -ne 0
# then
#   echo "error downloading $INTEL_MEDIA_DRIVER_SRC_NAME.tar.gz"
#   exit 1
# fi

rm -f $LIBVA_INTEL_DRIVER_SRC_NAME.tar.bz2
rm -f $LIBVA_INTEL_DRIVER_SRC_NAME.tar
wget https://github.com/intel/intel-vaapi-driver/releases/download/2.4.1/$LIBVA_INTEL_DRIVER_SRC_NAME.tar.bz2
if test $? -ne 0
then
  echo "error downloading $LIBVA_INTEL_DRIVER_SRC_NAME.tar.bz2"
  exit 1
fi

rm -f $INTEL_GMMLIB_SRC_NAME.tar.gz
wget https://github.com/intel/gmmlib/archive/refs/tags/$INTEL_GMMLIB_SRC_NAME.tar.gz
if test $? -ne 0
then
  echo "error downloading $INTEL_GMMLIB_SRC_NAME.tar.gz"
  exit 1
fi

rm -fr $LIBDRM_SRC_NAME
tar -zxf $LIBDRM_SRC_NAME.tar.gz
cd $LIBDRM_SRC_NAME
meson _build -Dprefix=$INSTALL_PATH $LIBDRM_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBDRM_SRC_NAME"
  exit 1
fi
ninja -C _build
if test $? -ne 0
then
  echo "error make $LIBDRM_SRC_NAME"
  exit 1
fi
cd _build
meson install
if test $? -ne 0
then
  echo "error make install $LIBDRM_SRC_NAME"
  exit 1
fi
cd ..
cd ..

rm -fr $LIBVA_SRC_NAME
bunzip2 -k $LIBVA_SRC_NAME.tar.bz2
tar -xf $LIBVA_SRC_NAME.tar
cd $LIBVA_SRC_NAME
meson _build -Dprefix=$INSTALL_PATH $LIBVA_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBVA_SRC_NAME"
  exit 1
fi
# this will get rid of libva info logging
echo "" >> config.h
echo "#define va_log_info(buffer)" >> config.h
echo "" >> config.h
ninja -C _build
if test $? -ne 0
then
  echo "error make $LIBVA_SRC_NAME"
  exit 1
fi
cd _build
meson install
if test $? -ne 0
then
  echo "error make install $LIBVA_SRC_NAME"
  exit 1
fi
cd ..
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
make -j"$(nproc)"
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

rm -rf gmmlib-$INTEL_GMMLIB_SRC_NAME
tar -xf $INTEL_GMMLIB_SRC_NAME.tar.gz
cd gmmlib-$INTEL_GMMLIB_SRC_NAME
mkdir _build
cd _build
cmake $INTEL_GMMLIB_CONFIG ..
if test $? -ne 0
then
  echo "error configure $INTEL_GMMLIB_SRC_NAME"
  exit 1
fi
make -j"$(nproc)"
if test $? -ne 0
then
  echo "error make $INTEL_GMMLIB_SRC_NAME"
  exit 1
fi
make install
if test $? -ne 0
then
  echo "error make install $INTEL_GMMLIB_SRC_NAME"
  exit 1
fi
cd ..
cd ..

git clone https://github.com/intel/media-driver.git media-driver-$INTEL_MEDIA_DRIVER_SRC_NAME

rm -rf build_media_driver || true
mkdir build_media_driver
cd build_media_driver
cmake $INTEL_MEDIA_DRIVER_CONFIG ../media-driver-$INTEL_MEDIA_DRIVER_SRC_NAME/
if test $? -ne 0
then
  echo "error cmake $INTEL_MEDIA_DRIVER_SRC_NAME"
  exit 1
fi
make -j"$(nproc)"
if test $? -ne 0
then
  echo "error make $INTEL_MEDIA_DRIVER_SRC_NAME"
  exit 1
fi
make install
if test $? -ne 0
then
  echo "error make install $INTEL_MEDIA_DRIVER_SRC_NAME"
  exit 1
fi
cd ..

rm -rf $LIBVA_INTEL_DRIVER_SRC_NAME
bunzip2 -k $LIBVA_INTEL_DRIVER_SRC_NAME.tar.bz2
tar -xf $LIBVA_INTEL_DRIVER_SRC_NAME.tar
cd $LIBVA_INTEL_DRIVER_SRC_NAME
meson _build -Dprefix=$INSTALL_PATH $LIBVA_INTEL_DRIVER_CONFIG
if test $? -ne 0
then
  echo "error configure $LIBVA_INTEL_DRIVER_SRC_NAME"
  exit 1
fi
ninja -C _build
if test $? -ne 0
then
  echo "error make $LIBVA_INTEL_DRIVER_SRC_NAME"
  exit 1
fi
cd _build
meson install
if test $? -ne 0
then
  echo "error make install $LIBVA_INTEL_DRIVER_SRC_NAME"
  exit 1
fi
cd ..
cd ..

rm -rf libyami
git clone https://github.com/intel/libyami.git
cd libyami
git checkout 1.3.2
./autogen.sh
CFLAGS="-O2 -Wall" CXXFLAGS="-O2 -Wall" ./configure --prefix=$INSTALL_PATH --libdir=$LIBRARY_INSTALLATION_DIR $LIBYAMI_CONFIG
if test $? -ne 0
then
  echo "error configure libyami"
  exit 1
fi
make clean
make -j"$(nproc)"
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
./configure --prefix=$INSTALL_PATH --libdir=$LIBRARY_INSTALLATION_DIR $LIBYAMI_INF_CONFIG
if test $? -ne 0
then
  echo "error configure yami_inf"
  exit 1
fi
make clean
make -j"$(nproc)"
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
