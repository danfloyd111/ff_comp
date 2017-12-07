#!/bin/bash

# Bash script used for displaying a spinner while waiting for the pipeline_benchmark to finish

handler(){
    echo " Killing comp_benchmark with PID: $PID"
    @kill  $PID
}

trap handler INT

$PWD/test/bin/comp_benchmark $@ &
PID=$!
i=0
sp='/-\|'
echo -n ' '
while [ -d /proc/$PID ]
do
    i=$(( (i+1) %4 ))
    printf "\r(${sp:$i:1}) "
    sleep .1
done