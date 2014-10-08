# on Franklin:
# type "module load acml" to use ACML (required)
# type "module swap PrgEnv-pgi PrgEnv-gnu" to use GNU compiler
# type "module swap PrgEnv-pgi PrgEnv-pathscale" to use PathScale compiler
#
# On other machines you might need to use other library than ACML
# this could be MKL on an Intel platform or ESSL on an IBM platform.
#
CC = icc

MKLROOT = /opt/intel/mkl
LIBS =  -Wall -O3 -L$(MKLROOT)/lib/intel64 -lmkl_intel_ilp64 -lmkl_core -lmkl_intel_thread -openmp -lpthread -lm
CFLAGS =  -g -no-vec -vec-report -Wall -O3 -DMKL_ILP64 -openmp -I$(MKLROOT)/include

INCLUDES = -I$(MKLROOT)/include

all:	verifyCounters

verifyCounters: verifyCounters.o CounterHomeBrew.o loops.h
	$(CC) -o $@ verifyCounters.o CounterHomeBrew.o $(LIBS)

%.o: %.c
	$(CC) $(INCLUDES) -c $(CFLAGS) $<
clean:
	rm -f verifyCounters *.o
