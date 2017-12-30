#!/bin/bash

# Author: Daniele Paolini <daniele.paolini@hotmail.it>
#
# This script executes a variable number of times a benchmark that aims to measure and spot any difference between ff_comp code
# and plain sequential code. Enabling "pipeline" flag with -p option will executes in addition ff_pipeline version of the
# benchmark, in order to measure the speedup granted by the parralel execution.
#
# It will print on stdout all the resulting statistics in a well readable format (the stdout may be redirected to a file to
# store the results).

function print_usage {
    printf "Usage: %s  runs [-p]\n" $1
    printf "    runs: number of times that benchmark will be executed\n"
    printf "    -p: executes pipeline test in addition to the others\n"
}

if [ $(( $# < 2 )) ]; then
    printf "Error: missing argument\n"
    print_usage $0
fi

# go on with getopts !
