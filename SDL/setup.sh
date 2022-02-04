#!/bin/sh

if [ ! -f setup.sh ]
then
  cd SDL
fi

if [ ! -f tileset.amiga ]
then
  for file in ../tileset.amiga ../PSP/level-* ../Amiga/*.png ../Music/mod.*
  do
    ln -s "$file"
  done
fi
