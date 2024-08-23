#!/usr/bin/env bash

RESULTS_DIR=${4:-./results}
mkdir -p $RESULTS_DIR

DATA_DIR=./data

function run_jitted() {
  N_THREADS=$1
  N_REPETITIONS=$2

  echo "##### Running with JITting..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."
    for format in {ttree,rntuple}; do
      echo "Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        if ! $(./clear_page_cache); then
          echo "Could not clear page cache. Make sure you have the right privileges."
          return 1
        fi
        results=$(root -q -l -b "./tasks/${task}/rdataframe_jitted_nanoaod.C(\"${format}\", \"${DATA_DIR}/nanoaod_53M.${format}.root\", ${N_THREADS})" 2>&1)
        echo "$results" >> $RESULTS_DIR/${task}_${format}_jitted_raw.txt
      done
    done
  done

  python benchmarks/get_times.py $RESULTS_DIR

  echo "All done!"
  return 0
}

function run_compiled() {
  N_THREADS=$1
  N_REPETITIONS=$2

  echo "### Running in compiled mode..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."

    if [[ ! -e "./tasks/${task}/compiled_nanoaod" ]]; then
      echo "Benchmark binary not found! Make sure to run 'make'"
      return 1
    fi

    for format in {ttree,rntuple}; do
      echo "   Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        if ! $(./clear_page_cache); then
          echo "Could not clear page cache. Perhaps run with sudo?"
          return 1
        fi
        results=$(./tasks/${task}/compiled_nanoaod -j ${N_THREADS} ${format} ${DATA_DIR}/nanoaod_53M.${format}.root 2>&1)
        echo "$results" >> $RESULTS_DIR/${task}_${format}_compiled_raw.txt
      done
    done
  done

  python benchmarks/get_times.py $RESULTS_DIR

  echo "All done!"
  return 0
}

function print_usage() {
  echo "Usage: $0 (jitted|compiled) N_THREADS N_REPETITIONS [RESULTS_DIR]"
}

if [[ "$#" -ne 3 ]]; then
  print_usage
  exit 1
fi

case $1 in
  jitted)
    run_jitted $2 $3
    ;;
  compiled)
    run_compiled $2 $3
    ;;
  *)
    print_usage
    exit 1
    ;;
esac
