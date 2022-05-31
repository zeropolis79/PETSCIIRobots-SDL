#!/bin/sh

if [ ! -f setup.sh ]
then
  cd Arcade
fi

if [ ! -f Data/GameScreen.raw.gz ]
then
  mkdir -p Data
  cd Data
  for file in ../../Amiga/Data/level-*.gz ../../Amiga/Data/mod.*.gz ../../Amiga/Data/Game*.gz
  do
    ln -s "$file"
  done
fi
