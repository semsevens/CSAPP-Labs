#!/usr/bin/bash

make CFLAGS='-O3 -g -DDRIVER' -B || exit

CORRECTNESS_CHECK_REPS="malloc malloc-free realloc short1 short2 corners"

for rep in $CORRECTNESS_CHECK_REPS; do
  echo "checking $rep"
  ./mdriver -c traces/${rep}.rep
  echo "$(printf '=%.0s' {1..120})"
done