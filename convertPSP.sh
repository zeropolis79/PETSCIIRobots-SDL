#!/bin/sh

# 32-bit textures, swizzled
for file in *.png
do
  psptextureconverter "${file}" "${file%.png}.psp" 3 0 1 1
done

