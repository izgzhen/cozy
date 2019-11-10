set -x
set -e

for target in ListSums SumMul; do
  for TIMEOUT in 30 90 150 300 600 900 1500; do
    rm -f $target.bin
    make $target.bin TIMEOUT=$TIMEOUT
  done
done
