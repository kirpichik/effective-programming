#!/bin/sh

for size in 1 2 4 8 16 32
do
  echo "$size byte(s):"
  time ./mem-test-$size
  echo "================"
done

