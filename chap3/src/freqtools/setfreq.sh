#!/bin/bash

for i in `seq 0 31`;
do
    sudo cpufreq-set -c $i -d $1 -u $1;
done;