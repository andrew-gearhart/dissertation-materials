#!/bin/bash

DATA_DIR=POWER_DATA
MACHINE=jkt
BENCHMARK=sgemv_hetero_blas

export MKL_DYNAMIC=FALSE 
export MKL_NUM_THREADS=15 
export OMP_NUM_THREADS=2 
export OMP_NESTED=TRUE 
export KMP_AFFINITY=compact,granularity=core

TIMESTAMP=`date +%F_%H_%M_%S`

RUNHEADER=${BENCHMARK}_${MACHINE}_${MKL_NUM_THREADS}_${OMP_NUM_THREADS}_${TIMESTAMP};

mkdir ${DATA_DIR}/${RUNHEADER};

minIter=2
minRuntime=20

# gemm
#Kd=200
#gpuOuter=5
#gpuInner=50
#for Kh in `seq 1000 1000 20000`;

#Kh=8000
#gpuOuter=5
#gpuInner=60
#for Kd in `seq 100 20 300`;

#gemv 
#Kd=600
#gpuOuter=25
#gpuInner=70
#for Kh in `seq 5000 2500 40000`;

Kh=15000
gpuOuter=25
gpuInner=175
for Kd in `seq 200 50 700`;
do 
    Mh=$Kh
    Nh=$Kh
    Md=$Kd
    Nd=$Kd
    SIZES=${Mh}_${Nh}_${Kh}_${Md}_${Nd}_${Kd}
    
    empty -f -L ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_${SIZES}.txt ./wattsup -t ttyUSB0 watts volts amps;
    sleep 5;
    ./${BENCHMARK} $gpuOuter $gpuInner $minIter $minRuntime $Md $Nd $Kd $Mh $Nh $Kh >> ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_counterData.txt;
    sleep 5;
    killall wattsup;
done

./extractHeteroPowerData.py ${RUNHEADER}
