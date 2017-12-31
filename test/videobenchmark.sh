#!/bin/bash

# Author: Daniele Paolini <daniele.paolini@hotmail.it>
#
# This script executes a variable number of times a benchmark that aims to measure and spot any difference between ff_comp code
# and plain sequential code. Enabling "pipeline" flag with -p option will executes in addition ff_pipeline version of the
# benchmark, in order to measure the speedup granted by the parralel execution.
#
# It will print on stdout all the resulting statistics in a well readable format (the stdout may be redirected to a file to
# store the results).
#
# Note: you have to run "make ffcompvideo" (or compile ffcompvideo.cpp) before running this script.

function print_usage {
    printf "Usage: %s [-p] runs\n" $1
    printf "    runs: number of times that benchmark will be executed\n"
    printf "    -p: executes pipeline test in addition to the others\n"
}

if [ $# -lt 1 ]; then
    printf "Error: missing argument\n"
    print_usage $0
    exit 1
fi

pipeline_flag=false

while getopts ":p" opt;  do
      case $opt in
	  p) printf "Selected pipeline mode in addition\n"
	     pipeline_flag=true;;
	  \?) printf "Error: illegal option -d\n" $OPTARG
	      print_usage $0
	      exit 1;;
      esac
done

shift $(($OPTIND -1))
runs=$1

if ! [[ "$runs" =~ ^[0-9]+$ ]]; then
    printf "Error: illegal argument %s\n" "$runs"
    print_usage $0
    exit 1
fi

for i in `seq 0 $((runs - 1))`; do
    printf "%d\n" $(( (i+1) / runs )) # print a percentual for every iteration
done
      
printf "Done!%d\n" $runs # remove this line in production
