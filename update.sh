#!/bin/bash

set -o errexit
trap 'echo "$0: error in line ${LINENO}."' ERR

CPU_CORES="$(grep -cEe '^processor' /proc/cpuinfo)"
SUDO=sudo
[[ "$(id -u)" == 0 ]] && SUDO=""


if [ "$1" == "clean" ]; then
   make clean
fi


#export CFLAGS="-g -ggdb -O2" \
#export CXXFLAGS="-g -ggdb -O2" \

export PATH="/opt/Octave/4.0.0/bin:$PATH"
export PKG_CONFIG_PATH="/opt/OpenCV/3.1.0/lib/pkgconfig:/opt/FFmpeg/57.27.101/lib/pkgconfig:$PKG_CONFIG_PATH"


./configure \
   --enable-ffmpeghack \
   LDFLAGS="-Wl,-rpath=/opt/OpenCV/3.1.0/lib"

make -j$((CPU_CORES + 1))

#make package

#sudo make install

#   --enable-staticffmpeg \
#   --with-altopath=/usr/lib/x86_64-linux-gnu/octave/3.8.1/oct/x86_64-pc-linux-gnu \
#   --with-mpath=/usr/share/octave/packages/video-$(cat VERSION) \
#   --with-opath=/usr/lib/x86_64-linux-gnu/octave/packages/video-$(cat VERSION)/x86_64-pc-linux-gnu-api-v48+ \
#   CFLAGS="-g -ggdb -gdwarf-3 -O3 -mtune=native" \
#   CPPFLAGS="-std=c++11 -g -ggdb -gdwarf-3 -O3 -mtune=native" \
#   CXXFLAGS="-std=c++11 -g -ggdb -gdwarf-3 -O3 -mtune=native" \

