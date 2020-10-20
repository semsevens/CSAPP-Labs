#!/usr/bin/bash

make CFLAGS='-g -DDRIVER -DDEBUG' -B

gdb --args ./mdriver -V -c $1