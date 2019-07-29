#!/bin/bash
for KEYSIZE in 1 4 8 16 32; do
  python changesize.py $KEYSIZE
  make workload_string
  mv workload_string workload_string${KEYSIZE}
done
