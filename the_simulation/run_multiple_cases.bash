#!/usr/bin/env bash

bedir=$1
if [[ -z "$bedir" ]]; then
    echo "No input dir was given. Exit."
    exit
fi
cd /mnt/c/polgarp/phd/simulation/the_simulation/$bedir
hgf=(`ls`)
cd /mnt/c/polgarp/phd/simulation/the_simulation
for i in "${hgf[@]}"; do
  { time ./the_simulation ./$bedir/"$i" 2>> errors.txt > ./$bedir/"$i".txt ; } 2>> times.txt
done
