from typing import List, Tuple
from argparse import ArgumentParser

import os
import re
import statistics

N_EVENTS = 100016

def get_rdf_wall_times(metricsLines: List[str]) -> List[str]:
    wall_times = []
    for ln in metricsLines:
        m = re.match(
            r"Info in <\[ROOT\.RDF\] Info (.*) in void "
            r"ROOT::Detail::RDF::RLoopManager::Run\(bool\)>: Finished event "
            r"loop number . \([\.0-9]*s CPU, (?P<wall_time>[\.0-9]*)s elapsed\)\.",
            ln,
        )
        if m:
            wall_times.append(float(m.group("wall_time")))

    return wall_times

def write_stats(stats: List[Tuple[float, ...]], path: str) -> None:
    with open(path, "w") as f:
        f.writelines("\t".join(line) + "\n" for line in stats)


if __name__ == "__main__":
    arg_parser = ArgumentParser(
        prog="extract_metrics.py",
        description="Extract relevant metrics from bm_readspeed output files",
    )
    arg_parser.add_argument("results_dir")
    args = arg_parser.parse_args()

    for root, dirs, files in os.walk(args.results_dir):
        for file in files:
            if not file.endswith(".txt"):
                continue

            path = os.path.join(root, file)

            with open(path, "r") as f:
                lines = f.readlines()
                wall_times = get_rdf_wall_times(lines)

            with open(path[:-4] + ".data", "w") as f:
                f.writelines(str(wt) + "\n" for wt in wall_times)
