#!/bin/bash
sudo chmod 666 /dev/cpu/*/msr
#sudo chmod 666 /dev/ttyUSB0
sudo cpufreq-set -g userspace
#../freqTools/setfreq.sh 2000000
../freqTools/setfreq.sh 2260000