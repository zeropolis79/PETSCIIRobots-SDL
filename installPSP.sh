#!/bin/sh

if [ `uname -s` = "Linux" ]
then
  MOUNTPOINT="/run/media/vesuri/disk"
else
  MOUNTPOINT="/cygdrive/g"
fi

make -f Makefile.PSP
if [ $? -ne 0 ]
then
  exit
fi
makeeboot petrobots
cp -v eboot.pbp ${MOUNTPOINT}/psp/game/petrobots/
if [ $? -ne 0 ]
then
  exit
fi
rm -f ${MOUNTPOINT}/log.txt
touch ${MOUNTPOINT}/log.txt
umount ${MOUNTPOINT}
