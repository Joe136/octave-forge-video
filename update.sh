#!/bin/bash

./configure \
   --with-mpath=/usr/share/octave/packages/video-1.0.2 \
   --with-altopath=/usr/lib/x86_64-linux-gnu/octave/3.6.4/oct/x86_64-pc-linux-gnu \
   --with-opath=/usr/lib/x86_64-linux-gnu/octave/packages/video-1.0.2/x86_64-pc-linux-gnu-api-v48+ \
\
&& \
\
make -j 8 \
\
&& \
\
sudo make install

