#!/bin/bash

for i in `seq 0 31`;
do
    sudo cpufreq-info -c $i $1;
done;
