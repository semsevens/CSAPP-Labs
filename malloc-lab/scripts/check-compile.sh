#!/usr/bin/bash

allMmCFiles=$(ls ./mm-*.c)
currentLinkedTarget=$(readlink -f mm.c)
currentLinkedFilename=$(basename $currentLinkedTarget)

for cFile in $allMmCFiles; do
  ln -sf ${cFile} mm.c
  make -B
  echo "$(printf '=%.0s' {1..120})"
done

ln -sf $currentLinkedFilename mm.c