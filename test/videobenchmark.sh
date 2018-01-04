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
# Note: place the video that you want to use in the same directory of this script.

function print_usage {
    printf "Usage: %s [-p] runs video\n" $1
    printf "    runs: number of times that benchmark will be executed\n"
    printf "    video: relative (to this script) path to the video\n"
    printf "    -p: executes pipeline test in addition to the others\n"
}

function join_by {
    local IFS=$1 # setting local internal field separator as first arg
    shift # getting rid of first arg
    echo "$*" # echoing the rest
}

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" # getting this script's directory

if [ $# -lt 2 ]; then
    printf "Error: missing argument\n"
    print_usage $0
    exit 1
fi

pipeline_flag=0

while getopts ":p" opt;  do
      case $opt in
	  p) printf "Selected pipeline mode in addition\n"
	     pipeline_flag=1;;
	  \?) printf "Error: illegal option -d\n" $OPTARG
	      print_usage $0
	      exit 1;;
      esac
done

shift $(($OPTIND -1))
runs=$1
video=$2

if ! [[ "$runs" =~ ^[0-9]+$ ]]; then
    printf "Error: illegal argument %s\n" "$runs"
    print_usage $0
    exit 1
fi

if [ $runs -lt 1 ]; then
    printf "Error: number of runs must be at least 1\n"
    print_usage $0
    exit 1
fi

printf "Starting the benchmark with %d iterations\n" $runs

for i in `seq 0 $((runs - 1))`; do
    printf "iteration number: %d\n" $i
    cd $THIS_DIR
    time=`./bin/ffcompvideo "$video" "0" | grep "Inner" | awk '{print $5}'` # comp run
    comp_array[i]=$time
    #./bin/ffcompvideo "$video" "1" # sequential run
    if [ $pipeline_flag -eq 1 ]; then
	bin/ffcompvideo "$video" "2" # optional pipeline run
    fi
    # TODO: print completion percentual and completion bar [#####------] 45% completed
done

# print average, min & max of comp test
sum=`join_by + ${comp_array[*]} | bc -l`
avg=`echo "scale=1; $sum / $runs" | bc -l`
printf "Average inner Comp completion time:\t%s\n" $avg
comp_min=${comp_array[0]}
comp_max=${comp_array[0]}
for i in "${comp_array[@]}"; do
    if (( $(echo "$comp_min > $i" | bc -l) )); then
	comp_min=$i
    fi
    if (( $(echo "$comp_max < $i" | bc -l) )); then
	comp_max=$i
    fi
done
printf "Minimum inner Comp completion time:\t%s\n" $comp_min
printf "Maximum inner Comp completion time:\t%s\n" $comp_max

printf "\n"
