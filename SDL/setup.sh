#!/bin/sh

if [ ! -f setup.sh ]
then
  cd SDL
fi

if [ ! -f tileset.amiga ]
then
  for file in ../tileset.amiga ../Music/mod.* ../Sounds/*.raw
  do
    ln -s "$file"
  done
fi
