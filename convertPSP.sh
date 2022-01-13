#!/bin/sh

for file in *.png
do
  psptextureconverter "${file}" "${file%.png}.psp" 3 0 1 0
done

