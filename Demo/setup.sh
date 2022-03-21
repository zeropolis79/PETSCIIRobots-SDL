#!/bin/sh

if [ ! -f setup.sh ]
then
  cd Demo
fi

if [ ! -f Data/IntroScreen.raw ]
then
  mkdir -p Data
  for file in ../Amiga/Data/level-a.gz ../Amiga/Data/level-d.gz "../Amiga/Data/mod.metal heads.gz" ../Amiga/Data/mod.win.gz ../Amiga/Data/mod.lose.gz "../Amiga/Data/mod.metallic bop amiga.gz" "../Amiga/Data/mod.rushin in.gz" ../Amiga/Data/Game*.gz ../Arcade/Data/Intro*.gz
  do
    output=`basename "$file"`
    gzip -cd "$file" > "Data/${output%.gz}"
  done
fi
