#!/usr/bin/env python3

import argparse as ap
import numpy as np
import scipy as sp
import matplotlib.pyplot as plt

g_args = None


def read_file(filename: str) -> list[float]:
    """Opens the file and parses the input"""
    ret = []
    try:
        with open(filename, "r") as inp:
            ret = [float(number) for number in inp.readlines() if number != ""]
    except IOError as e:
        exit(f"unable to open input file: {str(e)}")
    except Exception as e:
        exit(f"unable to open read_file {filename}: {str(e)}")
    return ret


def print_stats(vsyncs: list[float]) -> None:
    """Prints the statistics, of the file"""
    vsyncs = np.array(sorted(vsyncs))
    min = vsyncs[0]
    max = vsyncs[-1]

    median = sp.ndimage.median(vsyncs)
    mean = sp.ndimage.mean(vsyncs)
    print(f"min,max = {min},{max}\nmedian = {median}\nmean = {mean}")


def plot_stats(vsyncs: list[float]) -> None:
    """plots the histogram of the results"""
    plt.hist(vsyncs)
    plt.show()


def get_stats(filename: str, histogram: bool) -> None:
    """Get the stats from filename and optionally create a histogram"""
    vsyncs = np.array(read_file(filename))
    print_stats(vsyncs)
    if histogram:
        plot_stats(vsyncs)


def parse_cmd() -> None:
    global g_args
    parser = ap.ArgumentParser(
        "stats.py",
        description=(
            "Retrieve the stats of an output file of a vsync test program. The file"
            " should contain a single column with the times between successive vsyncs"
        ),
    )

    parser.add_argument("inputfile", type=str, help="The input file", nargs=1)
    parser.add_argument(
        "-p", "--plot", action="store_true", help="display plots/histograms"
    )
    g_args = parser.parse_args()


def main():
    parse_cmd()
    get_stats(g_args.inputfile[0], g_args.plot)


if __name__ == "__main__":
    main()
