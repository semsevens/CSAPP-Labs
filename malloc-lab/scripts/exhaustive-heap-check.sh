#!/usr/bin/bash

make CFLAGS='-O3 -g -DDRIVER -DCHECK_HEAP' -B || exit

REPS=$(ls traces/*.rep)

for rep in $REPS; do
  echo "checking $rep"
  ./mdriver -c $rep || exit
done

echo "$(printf '=%.0s' {1..120})"
echo "All heap checker passed!"