#!/usr/bin/env bash

N_REPETITIONS=$1

mkdir -p results

for task in $( seq 1 8 ); do
  for format in {ttree,rntuple}; do
    for i in $( seq 1 $N_REPETITIONS ); do
      clear_page_cache
      results=$(root -q -l "tasks/${task}/rdataframe_jitted_physlite.C(\"${format}\")" 2>&1)
      echo "$results" >> results/${task}_${format}_jitted.txt
    done
  done
done

python get_times.py results
