# on Franklin:
# type "module load acml" to use ACML (required)
# type "module swap PrgEnv-pgi PrgEnv-gnu" to use GNU compiler
# type "module swap PrgEnv-pgi PrgEnv-pathscale" to use PathScale compiler
#
# On other machines you might need to use other library than ACML
# this could be MKL on an Intel platform or ESSL on an IBM platform.
#
#CC = nvcc -ccbin /opt/intel/composer_xe_2013_sp1.1.106/bin/intel64/icpc
CC = nvcc -ccbin=icpc
MKLROOT = /opt/intel/mkl
#COUNTER_ROOT=/home/eecs/agearh/github/CounterHomeBrewOld
COUNTER_ROOT=../CHB
LIBS =  -L$(MKLROOT)/lib/intel64 -Xcompiler "-O3 -openmp" -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -lpthread -lm -lcublas
CFLAGS =  -O3 -Xcompiler "-Wall -O3 -mavx -opt-streaming-stores never -vec-report -xhost -openmp" -I$(MKLROOT)/include -m64
INCLUDES = -I$(MKLROOT)/include

all:	sgemm_hetero_blas sgemv_hetero_blas

# GEMM varients
sgemm_hetero_blas: sgemm_hetero_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@  $(LIBS) $(COUNTER_ROOT)/CounterHomeBrew.o sgemm_hetero_blas.o   


sgemv_hetero_blas: sgemv_hetero_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@  $(LIBS) $(COUNTER_ROOT)/CounterHomeBrew.o sgemv_hetero_blas.o   

%.o: %.cpp
	$(CC) $(INCLUDES) -c $(CFLAGS) $<
clean:
	rm -f sgemm_hetero_blas sgemv_hetero_blas *.o
