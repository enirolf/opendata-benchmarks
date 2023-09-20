#!/usr/bin/env bash

function run_jitted() {
  N_REPETITIONS=$1

  mkdir -p results

  echo "##### Running in JITted mode..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."
    for format in {ttree,rntuple}; do
      echo "Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        clear_page_cache
        results=$(root -q -l -b "tasks/${task}/rdataframe_jitted_physlite.C(\"${format}\")" 2>&1)
        echo "$results" >> results/${task}_${format}_jitted.txt
      done
    done
  done

  python get_times.py results

  echo "All done!"
  return 0
}

function run_compiled() {
  N_REPETITIONS=$1

  mkdir -p results

  echo "### Running in compiled mode..."

  for task in $( seq 1 8 ); do
    echo "*** Running task $task..."

    if [[ ! -e "tasks/${task}/compiled_physlite" ]]; then
      echo "Benchmark binary not found! Make sure to run 'make'"
      return 1
    fi

    for format in {ttree,rntuple}; do
      echo "   Running with ${format}..."
      for i in $( seq 1 $N_REPETITIONS ); do
        clear_page_cache
        results=$(./tasks/${task}/compiled_physlite ${format} 2>&1)
        echo "$results" >> results/${task}_${format}_compiled.txt
      done
    done
  done

  python get_times.py results

  echo "All done!"
  return 0
}

function print_usage() {
  echo "Usage: $0 {jitted|compiled} N_REPETITIONS"
}

if [[ -z "$2" ]]; then
  print_usage
  exit 1
fi

case $1 in
  jitted)
    run_jitted $2
    ;;
  compiled)
    run_compiled $2
    ;;
  *)
    print_usage
    exit 1
    ;;
esac

