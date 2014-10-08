CC = icc -g

MKLROOT = /opt/intel/mkl
COUNTER_ROOT=../CHB
LIBS =  -Wall -L$(MKLROOT)/lib/intel64 -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -openmp -lpthread -lm
CFLAGS =  -Wall -O3 -mavx -opt-streaming-stores never -vec-report -xhost -ipo -openmp -I$(MKLROOT)/include
SPARSELIBS= -L./bebop_util -L./sparse_matrix_converter -lbebop_util -lsparse_matrix_converter
SPARSEINCLUDES= -I./bebop_util/include -I./sparse_matrix_converter/include

INCLUDES = -I$(MKLROOT)/include -I$(COUNTER_ROOT)

all:	dgemm_naive dgemm_blas dgemv_naive dgemv_blas

# SPMV
dspmvBLASCSR: dspmv_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dspmv_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS) $(SPARSELIBS)
dspmv_blas.o: dspmv_blas.c
	$(CC) $(INCLUDES) $(SPARSEINCLUDES) -c $(CFLAGS) $<
dspmvNaiveCSR: dspmv_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dspmv_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS) $(SPARSELIBS)
dspmv_naive.o: dspmv_naive.c
	$(CC) $(INCLUDES) $(SPARSEINCLUDES) -c $(CFLAGS) $<
# GEMM varients
dgemm_blas: dgemm_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dgemm_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS)
dgemm_naive: dgemm_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dgemm_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS)

# GEMV varients
dgemv_blas: dgemv_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dgemv_blas.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS)
dgemv_naive: dgemv_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o
	$(CC) -o $@ dgemv_naive.o $(COUNTER_ROOT)/CounterHomeBrew.o $(LIBS)

%.o: %.c
	$(CC) $(INCLUDES) -c $(CFLAGS) $<
clean:
	rm -f dgemm_naive dgemm_blas dgemv_naive dgemv_blas dspmvBLASCSR dspmvNaiveCSR *.o
