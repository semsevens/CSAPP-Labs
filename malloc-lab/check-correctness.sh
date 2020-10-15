#!/usr/bin/bash

make CFLAGS='-O2 -g -DDRIVER' -B

CORRECTNESS_CHECK_REPS="malloc malloc-free realloc short1 short2 corners"

for rep in $CORRECTNESS_CHECK_REPS; do
  ./mdriver -c traces/${rep}.rep
done