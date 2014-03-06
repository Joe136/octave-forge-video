#! /bin/sh -f

if [ -e src/autogen.sh ]; then
  cd src
  ./autogen.sh $*
fi

