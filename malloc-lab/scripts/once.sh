#!/usr/bin/bash

make -B || exit

./mdriver -V -c $1