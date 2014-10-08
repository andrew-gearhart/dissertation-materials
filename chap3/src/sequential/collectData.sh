#!/bin/bash

USE_HUGE_PAGES=no
PREFETCHERS=yes
DATA_DIR=POWER_DATA
MACHINE=emerald
BENCHMARK=dgemm_naive
export OMP_NUM_THREADS=32
export KMP_AFFINITY=compact,granularity=core


if [ "${BENCHMARK}" = 'dspmvBLASCSR' ] || [ "${BENCHMARK}" = 'dspmvNaiveCSR' ]; then
    echo "SPMV requested"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./sparse_matrix_converter:./bebop_util;
    MATRIXROOT=/nscratch/agearh/allMTX
fi

TIMESTAMP=`date +%F_%H_%M_%S`

m=2000; 
n=2000;
if [ "${USE_HUGE_PAGES}" = 'yes' ]; then
    RUNHEADER=${BENCHMARK}_${MACHINE}_${OMP_NUM_THREADS}_HUGEPAGE_${TIMESTAMP};
elif [ "${PREFETCHERS}" = 'no' ]; then
    RUNHEADER=${BENCHMARK}_${MACHINE}_${OMP_NUM_THREADS}_NOPREFETCH_${TIMESTAMP};
else
    RUNHEADER=${BENCHMARK}_${MACHINE}_${OMP_NUM_THREADS}_${TIMESTAMP};
fi

mkdir ${DATA_DIR}/${RUNHEADER};
# gemm
for k in `seq 1000 100 4000`;
#for k in `seq 2000 1000 15000`;
#gemv 
#for k in `seq 2500 1000 25000`;
# DSPMV
#for FILE in `dir -I "*_b.mtx" ${MATRIXROOT}`;
#for FILE in `dir /nscratch/agearh/allMTX`
#for FILE in 144.mtx
do 
    m=$k
    n=$k
    # in the sparse case, need to extract m,n,nnz from file to create power file (first line in file without '%'...probably tack in filename as well...
    if [ "${BENCHMARK}" = 'dspmvBLASCSR' ] || [ "${BENCHMARK}" = 'dspmvNaiveCSR' ]; then
#    if [ "${BENCHMARK}" = 'dspmvBLASCSR' ]; then
	n=1
	k=1
        SIZES=`./getSparseSizes.py ${MATRIXROOT}/${FILE}`;
    elif [ "${BENCHMARK}" = 'verifyCountersParallel' ]; then
	TEST=3
	MINRUNTIME=10
	SIZES=${k}_${k}_${TEST}
	m=$k
	n=${TEST}
	k=${MINRUNTIME}
    else
	SIZES=${m}_${n}_${k}
    fi
    
    empty -f -L ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_${SIZES}.txt ./wattsup -t ttyUSB0 watts volts amps;
    sleep 5;
    if [ "${USE_HUGE_PAGES}" = 'yes' ]; then
	if [ "${BENCHMARK}" = 'dspmvBLASCSR' ] || [ "${BENCHMARK}" = 'dspmvNaiveCSR' ]; then
#	if [ "${BENCHMARK}" = 'dspmvBLASCSR' ]; then
	    LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=yes numactl --interleave=all ./${BENCHMARK} ${FILE} $n $k 2 >> ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_counterData.txt;
	else
	    LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=yes ./${BENCHMARK} $m $n $k 2 >> ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_counterData.txt;
	fi
    else
	if [ "${BENCHMARK}" = 'dspmvBLASCSR' ] || [ "${BENCHMARK}" = 'dspmvNaiveCSR' ]; then
#	if [ "${BENCHMARK}" = 'dspmvBLASCSR' ]; then
	    numactl --interleave=all ./${BENCHMARK} ${FILE} $n $k 2 >> ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_counterData.txt;
	else
	    ./${BENCHMARK} $m $n $k 5 >> ${DATA_DIR}/${RUNHEADER}/${RUNHEADER}_counterData.txt;
	fi
    fi
    sleep 5;
    killall wattsup;
done

python extractPowerData.py ${RUNHEADER} no
