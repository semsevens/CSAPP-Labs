#!/usr/bin/bash

make CFLAGS='-O3 -g -DDRIVER -DDEBUG -DPRINT_HEAP -DCHECK_HEAP' -B || exit

./mdriver -V -c $1
