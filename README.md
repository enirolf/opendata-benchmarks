# Sofware benchmarks with open data

This branch contains two versions of the RDataFrame implementation for NanoAOD. One version includes just-in-time compilation and the other is fully compiled and therefore optimized for performance. Also, running the files as C++ macro, via the ROOT interpreter cling, will result in a reduced runtime performance due to missing optimizations. To compile any of the benchmarks named `*_compiled.cxx`, source ROOT and run `make`.

Each task will produce a histogram with a result of the computation. Reference histograms are included in this repository to verify each benchmark task produces the expected results.

To run the macros (named `{TASK}_jitted.C`) efficiently with optimizations, please use the following command:

```sh
root -l -b -q {TASK}_jitted.C+
```

All benchmarks can be run in batch mode with the `time_task.sh` script in the `benchmarks/` directory.
It takes three arguments: whether to run the JITted (use 'jitted') or compiled (use 'compiled') version of the benchmark task, the number of threads to use, and the number of times to repeat each benchmark task. For example, to run the compiled version, using 4 threads with 5 repetitions:
```sh
./benchmarks/time_task.sh compiled 4 5
```

The data files when using this script are expected to be in the `data/` directory in the root of this repository, and are expected to be named `nanoaod_53M.ttree.root` and `nanoaod_53.rntuple.root`, for TTree and RNTuple, respectively. See the last paragraph for more information on how to obtain this data.

> [!IMPORTANT]
> For multiple repetitions, it is important to clear the page cache between runs to ensure we are reading the data 'cold'. For this purporse, the `clear_page_cache` utility is provided, which can be built with `make clear_page_cache`. It is required to be present for batch-running the benchmarks.

Results are written to `results/`, where for each benchmark task there are two files -- one with the raw output, and one where the runtimes have been extracted. The results can be plotted using ROOT with the following command:
```sh
 root -l -q -b benchmarks/plot_throughput_nanoaod.C
 ```
 These plots are saved as PDF in `results/` as well.

## Benchmark tasks

1. Plot the Missing ET in an event (loop over events)
2. Plot the Jet pT of all jets in an event (loop over an array in an event)
3. Plot the Jet pT with eta cuts on the jet (loop over an array that is filtered)
4. Plot the missing ET of events with at least 2 jets above 40 GeV (loop over array in event, aggregate those results to filter at event level).
5. Plot missing et for events that have an opposite-sign muon pair of mass 60-120 GeV [looping on combinations of objects in one collection, 4-vector algebra]
6. Plot the pt of the tri-jet system with mass closest to 172.5 GeV, and the leading b-tag discriminator among the 3 jets in the triplet [requires looping on combination of objects in the same collection, and extracting properties of a combination other than the key used to sort them]
7. Plot sum of the pt of all jets of pt > 30 GeV that are not within DR 0.4 from a lepton of pt > 10 GeV [requires looping on two separate collections]
8. In events with >=3 leptons and a same-flavour opposite-sign lepton pair, find the best same-flavour opposite-sign lepton pair (mass closest to 91.2 GeV), and plot the transverse mass of the missing energy and the leading other lepton [ a task whose formulation in an imperative language is easy, but whose translations to a functional query language may be less clear and/or possibly inefficient]

## NanoAOD data sets

[Based on the following dataset, converted to NanoAOD](https://github.com/cms-opendata-analyses/AOD2NanoAODOutreachTool) from [2012 CMS open data](http://opendata.cern.ch/record/6021):
  * root://eospublic.cern.ch//eos/root-eos/benchmark/Run2012B_SingleMu.root (16 GiB, 53 million events)

A TTree and RNTuple based on this data set, both compressed using Zstd is available upon request. This RNTuple is written according to the data format specification of ROOT v6.32.
