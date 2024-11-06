#!/bin/sh

for f  in `ls *.png`; do
  mogrify -format jpg -resize 2048x2048\> $f
  rm $f
done

