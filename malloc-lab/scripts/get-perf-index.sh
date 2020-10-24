#!/usr/bin/bash

linkedTarget=$(readlink -f mm.c)
filename=$(basename $linkedTarget .c)

make CFLAGS='-O3 -g -DDRIVER -DUSE_MACRO -DUSE_GCC_BUILTIN' -B || exit
./mdriver | tee results/$filename.txt