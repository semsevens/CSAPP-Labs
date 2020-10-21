#!/usr/bin/bash

linkedTarget=$(readlink -f mm.c)
filename=$(basename $linkedTarget .c)

make CFLAGS='-O3 -g -DDRIVER' -B
./mdriver | tee results/$filename.txt