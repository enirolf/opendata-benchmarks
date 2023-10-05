#!/usr/bin/env bash

RESULTS_DIR=${4:-./results}
mkdir -p $RESULTS_DIR

function run_jitted() {
  EDM=$1
  N_REPETITIONS=$2

  echo "##### Running in JITted mode..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."
    for format in {ttree,rntuple}; do
      echo "Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        clear_page_cache
        results=$(root -q -l -b "tasks/${task}/rdataframe_jitted_${EDM}.C(\"${format}\")" 2>&1)
        echo "$results" >> $RESULTS_DIR/${task}_${format}_jitted.txt
      done
    done
  done

  python get_times.py $RESULTS_DIR

  echo "All done!"
  return 0
}

function run_compiled() {
  EDM=$1
  N_REPETITIONS=$2

  echo "### Running in compiled mode..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."

    if [[ ! -e "tasks/${task}/compiled_${EDM}" ]]; then
      echo "Benchmark binary not found! Make sure to run 'make'"
      return 1
    fi

    for format in {ttree,rntuple}; do
      echo "   Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        clear_page_cache
        results=$(./tasks/${task}/compiled_nanoaod ${format} 2>&1)
        echo "$results" >> $RESULTS_DIR/${task}_${format}_compiled.txt
      done
    done
  done

  python get_times.py $RESULTS_DIR

  echo "All done!"
  return 0
}

function print_usage() {
  echo "Usage: $0 {nanoaod|physlite} {jitted|compiled} N_REPETITIONS [RESULTS_DIR]"
}

if [[ -z "$2" ]]; then
  print_usage
  exit 1
fi

case $1 in
  nanoaod|physlite)
    ;;
  *)
    print_usage
    exit 1
    ;;
esac


case $2 in
  jitted)
    run_jitted $1 $3
    ;;
  compiled)
    run_compiled $1 $3
    ;;
  *)
    print_usage
    exit 1
    ;;
esac

