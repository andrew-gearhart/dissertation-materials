#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <immintrin.h>
#include <assert.h>
#include <bebop/smc/sparse_matrix.h>
#include <bebop/smc/sparse_matrix_ops.h>
#include <bebop/smc/csr_matrix.h>

#include <CounterHomeBrew.h>
#include <omp.h>
#include <mkl.h>

#define FUNC naiveSpmv

char* path = "/nscratch/agearh/allMTX/";

char* getTimeStamp(void){
  char fmt[64];
  char *buf = (char*)malloc(64*sizeof(char));
  struct timeval  tv;
  struct tm       *tm;

  gettimeofday(&tv, NULL);
  if((tm = localtime(&tv.tv_sec)) != NULL)
    {
      //      strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%06u %z", tm);
      strftime(fmt, sizeof fmt, "[%H:%M:%S]", tm);
      snprintf(buf, 64, fmt, tv.tv_usec);
    }
  return buf;
}

double read_timer( )
{
    static int initialized = 0;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = 1;
    }

    gettimeofday( &end, NULL );

    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double randDouble(double high, double low) {
  return (low + rand() * (high - low) / RAND_MAX);
}

void fill( double *p, int n )
{
  int i;
#pragma omp parallel for private(i)
  for( i = 0; i < n; i++ )
     p[i] = randDouble(2,1);
}

// END HELPER FUNCTIONS/DEFINITIONS
//  FUNC(&trans,&M,(double*)A->values,A->rowptr,A->colidx,b,c);
void naiveSpmv(char *transa, int *m, double *a, int *ia, int *ja, double *x, double *y){
  int i,j,k;
  double yi;
  // independent parallelism, but strided access to A...
#pragma omp parallel for private(i,j,yi)
  for (i = 0; i < *m; i++) {
    yi = 0;
    for (j = ia[i]; j < ia[i+1]; j++) {
      yi += a[j]*x[ja[j]];
    }
    y[i] = yi;
  }
}

int main(int argc, char *argv[]) {
  int i,j,k;
  char *ts1,*ts2;
  machineInformation currentMachine;
  counterSessionInfo session;
  double seconds = 0.0;

  // Set machine information from CounterHomeBrew.h
  currentMachine.cpu_model = CPU_MODEL;
  currentMachine.num_sockets = NUM_SOCKETS;
  currentMachine.num_phys_cores_per_socket = NUM_PHYS_CORES_PER_SOCKET;
  currentMachine.num_cores_per_socket = NUM_CORES_PER_SOCKET;
  currentMachine.num_cores = NUM_CORES;
  currentMachine.num_cbos = NUM_PHYS_CORES_PER_SOCKET; // should multiply by NUM_SOCKETS???
  currentMachine.core_gen_counter_num_max = CORE_GEN_COUNTER_MAX;
  currentMachine.cbo_counter_num_max = CBO_COUNTER_NUM_MAX;

  // NHM-EX
  session.core_gen_counter_num_used = 0;
  int32 core_event_numbers[] = {};
  int32 core_umasks[] = {};

  session.cbo_counter_num_used = 1;
  int32 cbo_event_numbers[] = {0x14};
  int32 cbo_umasks[] = {0x7};

  // JKT
  /*
  session.core_gen_counter_num_used = 5;
  int32 core_event_numbers[] = {0x10,0x10,0x11,0x51,0xF1};
  int32 core_umasks[] = {0x80,0x10,0x02,0x01, 0x07};

  session.cbo_counter_num_used = 1;
  int32 cbo_event_numbers[] = {0x37};
  int32 cbo_umasks[] = {0xf};
  session.cbo_filter = 0x1f;
  */
  for (i = 0; i < session.core_gen_counter_num_used; i++) {
    session.core_event_numbers[i] = core_event_numbers[i];
    session.core_umasks[i] = core_umasks[i];
  }
  for (i = 0; i < session.cbo_counter_num_used; i++) {
    session.cbo_event_numbers[i] = cbo_event_numbers[i];
    session.cbo_umasks[i] = cbo_umasks[i];
  }

  int fd[NUM_CORES];

  // Arrays to hold counter data...
  counterData before;
  counterData after;
  
  double minRuntime = 10.0;
  int M,N,K,actualNNZ;
  int info = -1;
  double alpha=1.0,beta=1.0;
  struct sparse_matrix_t* Awrap = NULL;
  struct csr_matrix_t* A = NULL;
  char filename[50];

  sprintf(filename,"%s%s",path,argv[1]);
  // does root not have enough power to do this?
  Awrap = load_sparse_matrix(MATRIX_MARKET,filename);
  if (Awrap == NULL) exit(-1);
  if (Awrap->repr == NULL) exit(-1);
 //  assert(Awrap != NULL);
  //  assert(Awrap->repr != NULL);
  info = sparse_matrix_convert (Awrap, CSR);
  if (info != 0) {destroy_sparse_matrix (Awrap);exit(-1);}

  A = Awrap->repr;
  K=A->nnz;
  M = csr_matrix_num_rows(A);
  N = csr_matrix_num_cols(A);
  if (M != N) exit(-1);
  //  assert(M == N);
  info = csr_matrix_expand_symmetric_storage(A);
  if (info != 0) {destroy_sparse_matrix (Awrap);exit(-1);}

  if(A->values == NULL) {
    double* tmp;
    A->values = (void*)malloc(sizeof(double)*A->nnz);
    tmp = (double*)(A->values);
    for (i = 0; i < A->nnz; i++)
      tmp[i] = 1.0;
  }
  actualNNZ=A->nnz;

  // assume some dummy values in argv[2],argv[3]...
  uint64 min_iters = strtoull(argv[4],NULL,0);

  double *b = (double*)calloc(N,sizeof(double));
  double *c = (double*)calloc(N,sizeof(double));
  char trans = 'N';
  for (i = 0;i< N;i++) b[i] = 1.0;
  

 // open the msr files for each core on the machine
  for (i = 0; i < currentMachine.num_cores; i++)
    open_msr_file(i,&fd[i]);

  // warm up da caches...
  // BLASFUNC( CblasColMajor,CblasNoTrans, CblasNoTrans,M,N,K, 1, A,M, B,K, 1, C,M );

  // Program the counters!!!
  int socketsProgrammed = 0;
  for (i = 0; i < currentMachine.num_cores; i++) {
    int currentCoreFD = fd[i];
    
    /* clear global control register before programming */
    stopCounters(i, currentCoreFD, &currentMachine, &session);

    /* set up the fixed counters on each core */
    programCoreFixedCounters(currentCoreFD);
    
    /* set up the general purpose registers for each core */
    programGeneralPurposeRegisters(currentCoreFD, &currentMachine, &session);

    /* Program the Uncore as desired...*/
    // Only program the first physical core on each socket. 
    // NOTE: Some assumptions about topology here...check /proc/cpuinfo to confirm.
 #if CPU_MODEL == JAKETOWN
    if (i % currentMachine.num_phys_cores_per_socket == 0 && socketsProgrammed < currentMachine.num_sockets)
#elif CPU_MODEL == NEHALEM_EX
    if (i < currentMachine.num_sockets && socketsProgrammed < currentMachine.num_sockets)
#elif CPU_MODEL == IVY_BRIDGE
    if (i < currentMachine.num_sockets && socketsProgrammed < currentMachine.num_sockets)
#endif
      {
	programUncoreCounters( currentCoreFD, &currentMachine, &session);
	socketsProgrammed++;
      }

    /* set global control register to active counters */
    //    startCounters( i, currentCoreFD, &currentMachine, &session);
  }
  
  uint64 num_iters = min_iters;
  for (num_iters = min_iters; seconds < minRuntime; num_iters *=2) {
    assert(A->values != NULL);
    assert(A->rowptr != NULL);
    assert(A->colidx != NULL);
 
    if (num_iters != min_iters) {
      free(ts1);
      free(ts2);
    }
    sleep(5);
    seconds = 0.0;

    // start the programmed counters...
    for (i = 0; i < currentMachine.num_cores; i++)
      startCounters( i, fd[i], &currentMachine, &session);
    
    /* READ COUNTERS BEFORE STUFF */
    readCounters(fd,&currentMachine,&session, &before);
    ts1 = getTimeStamp();
    seconds = read_timer();

    /* DO STUFF */    
    //    printf("num_iters = %"PRIu64"\n",num_iters);
    for (i =0; i < num_iters; i++)
      // WARNING: Don't try to use with 64-bit ints!!! Weird segfaults!!!
      FUNC(&trans,&M,(double*)A->values,A->rowptr,A->colidx,b,c);
    /* END DOING STUFF */

    seconds = read_timer()-seconds;
    ts2 = getTimeStamp();

    /* READ COUNTERS AFTER STUFF */    
    for (i = 0; i < currentMachine.num_cores; i++)
      stopCounters(i,fd[i],&currentMachine, &session);
    
    //  printf("num_iters = %"PRIu64", runtime is %g\n",num_iters,seconds);
  }
  num_iters /= 2;

  readCounters(fd,&currentMachine,&session,&after);
  diffCounterData(&currentMachine, &session, &after, &before, &after);

  uint64 *coreSums;
  coreSums = (uint64*)calloc(currentMachine.num_sockets*session.core_gen_counter_num_used,sizeof(uint64));
  for (i = 0; i < currentMachine.num_sockets; i++) {
    for (j = 0; j < currentMachine.num_cores_per_socket; j++) {
      for (k = 0; k < session.core_gen_counter_num_used; k++)
	// bug in the indexing of the core sums???
	//        coreSums[i*session.core_gen_counter_num_used + k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
        coreSums[k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
    }
  }

  uint64 *sums;
  sums = (uint64*)calloc(currentMachine.num_sockets*session.cbo_counter_num_used,sizeof(uint64));
  for (i = 0; i < currentMachine.num_sockets; i++) {
    //    printf("Socket %d\n",i);
    for (j = 0; j < currentMachine.num_cbos; j++) {
      //   printf("\tCbo %d\n\t",j);
      for (k = 0; k < session.cbo_counter_num_used; k++) {
	//	printf("%"PRIu64",",after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k]);
	// bug in the indexing of the core sums???
	//        sums[i*session.cbo_counter_num_used + k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
        sums[k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
      }
      // printf("\n");
    }
  }

  printf("%s,%s,%"PRIu64",%d,%d,%d,%f,",ts1,ts2,num_iters,M,N,K,seconds/(double)num_iters);
    for (j = 0; j < session.core_gen_counter_num_used; j++)
      //      printf("%"PRIu64",",after.generalCore[0][j]);
      printf("%f,",coreSums[j]/(double)num_iters);
    for (j = 0; j < session.cbo_counter_num_used; j++)
      printf("%f,",sums[j]/(double)num_iters);
    printf("%s,%d,",argv[1],actualNNZ);
    printf("\n");

  free(sums);
  free(coreSums);

  // Stop counters, reset PMU, close msr files
  cleanup(fd,&currentMachine,&session);

  destroy_sparse_matrix(Awrap);
  free(b);
  free(c);
      
  return 0;
}
