"""
get_times.py

usage: get_times.py [-h] results_dir

Extract RDF JIT and wall times from ROOT log files

Author: Florine de Geus <florine.de.geus@cern.ch>
"""


from typing import List, Tuple
from argparse import ArgumentParser

import os
import re
import statistics


def get_ttree_read_rates(metricsLines: List[str], dataset, task, f_out):
    read_rates = []
    for ln in metricsLines:
        m = re.match(r"Disk IO\s*=\s*(?P<read_rate>[\.0-9]*) MBytes/s", ln)
        if m:
            f_out.write(f"{dataset} ttree {task} {m.group('read_rate')}\n")
            read_rates.append(m.group("read_rate"))
    print(read_rates)


    # return statistics.mean(read_rates), statistics.stdev(read_rates)


def get_rntuple_read_rates(metricsLines: List[str], dataset, task, f_out):
    read_rates = []
    for ln in metricsLines:
        m = re.match(r"RDF\.RPageSourceFile\.bwRead\|MB/s\|bandwidth compressed bytes read per second\|(?P<read_rate>[\.0-9]*)", ln)
        if m:
            f_out.write(f"{dataset} rntuple {task} {m.group('read_rate')}\n")
            read_rates.append(m.group("read_rate"))
    print(read_rates)


    # return statistics.mean(read_rates), statistics.stdev(read_rates)


if __name__ == "__main__":
    arg_parser = ArgumentParser(
        prog="get_read_stats.py",
        description="Extract TTree and RNTuple read statistics",
    )
    arg_parser.add_argument("results_dir")
    args = arg_parser.parse_args()

    with open(os.path.join(args.results_dir, "read_rates.data"), "w") as f_out:
        for task in range(1, 9):
            for dataset in ["physlite", "nanoaod"]:
                with open(os.path.join(args.results_dir, f"{dataset}/{task}_ttree_compiled.txt")) as f_ttree:
                    lines = f_ttree.readlines()
                    # f_out.write(f"{dataset},{task},")
                    get_ttree_read_rates(lines, dataset, task, f_out)

                with open(os.path.join(args.results_dir, f"{dataset}/{task}_rntuple_compiled.txt")) as f_rntuple:
                    lines = f_rntuple.readlines()
                    # f_out.write(f"{dataset},{task},")
                    get_rntuple_read_rates(lines, dataset, task, f_out)

# [29.219, 46.457, 45.059, 39.295, 45.093, 45.312, 44.874, 45.186, 45.996, 45.674]
# 1182222
