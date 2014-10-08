#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <immintrin.h>

#include <CounterHomeBrew.h>
//#include <omp.h>
#include <mkl.h>

char* getTimeStamp(void){
  char fmt[64];
  char *buf = (char*)malloc(64*sizeof(char));
  struct timeval  tv;
  struct tm       *tm;

  gettimeofday(&tv, NULL);
  tm = localtime(&tv.tv_sec);
  if(tm != NULL)
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

void doNaiveMatmul(uint64 M,uint64 N,uint64 K, double* A, double* B, double* C) {  
  int i,j,k;

#pragma omp parallel for private(i,j,k)
  for (k = 0; k < K; k++) {
  for (i = 0; i < M; i++) {
  for (j = 0; j < N; j++) {
	  C[i+j*M] += A[i+k*M]*B[k+j*K];
	}
      }
    }

  /*
  __m256d a,b,c;
#pragma omp parallel for private(i,j,k,a,b,c)
  for (j = 0; j < N/4; j++) 
    for (k = 0; k < K/4; k++) {
      //      double bkj = B[k+j*K];
      b = _mm256_loadu_pd(B+4*k+j*K);
      for (i = 0; i < M/4; i++) {
	a = _mm256_loadu_pd(A+4*i+k*M);
	c = _mm256_loadu_pd(C+4*i+j*M);
	a = _mm256_mul_pd(a,b);
	c = _mm256_add_pd(a,c);
	_mm256_storeu_pd(C+4*i+j*M,c);
	//	C[i+j*M] += A[i+k*M]*bkj;
      }
    }
  */
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
  currentMachine.num_cbos = NUM_PHYS_CORES_PER_SOCKET;
  currentMachine.core_gen_counter_num_max = CORE_GEN_COUNTER_MAX;
  currentMachine.cbo_counter_num_max = CBO_COUNTER_NUM_MAX;

  // Set session events, umasks and counters use
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

  // some data for doing a naive matmul to test flop counting...
  // initloop(N);
  
  uint64 min_iters = 5;
  double minRuntime = 10.0;
  int M = atoi(argv[1]);
  int N = atoi(argv[2]);
  int K = atoi(argv[3]);
  double *A = NULL;
  double *B = NULL;
  double *C = NULL;
  //  posix_memalign((void**)A,64,M*K*sizeof(double));
  // posix_memalign((void**)B,64,K*N*sizeof(double));
  // posix_memalign((void**)C,64,M*N*sizeof(double));
  A = (double*) malloc( M * K * sizeof(double) );
  B = (double*) malloc( K * N * sizeof(double) );
  C = (double*) malloc( M * N * sizeof(double) );      
  fill( A, M * K );
  fill( B, K * N );
  fill( C, M * N );
  

 // open the msr files for each core on the machine
  for (i = 0; i < currentMachine.num_cores; i++)
    open_msr_file(i,&fd[i]);

  // warm up da caches...
  doNaiveMatmul(M,N,K,A,B,C);

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
  
  uint64 num_iters;
  for (num_iters = min_iters; seconds < minRuntime; num_iters *=2) {
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
    for (i =0; i < num_iters; i++)
      doNaiveMatmul(M,N,K,A,B,C);
    /* END DOING STUFF */

    seconds = read_timer()-seconds;
    ts2 = getTimeStamp();

    /* READ COUNTERS AFTER STUFF */    
    for (i = 0; i < currentMachine.num_cores; i++)
      stopCounters(i,fd[i],&currentMachine, &session);
    
  }
  num_iters /= 2;

  readCounters(fd,&currentMachine,&session,&after);
  diffCounterData(&currentMachine, &session, &after, &before, &after);

  uint64 *coreSums;
  coreSums = (uint64*)calloc(currentMachine.num_sockets*session.core_gen_counter_num_used,sizeof(uint64));
  
  for (i = 0; i < currentMachine.num_sockets; i++) {
    for (j = 0; j < currentMachine.num_cores_per_socket; j++) {
      for (k = 0; k < session.core_gen_counter_num_used; k++) {
	//	if (k == 0)
	//  printf("Socket %d, Core %d, DP scalar ops: %"PRIu64"\n",i,j,after.generalCore[i*currentMachine.num_cores_per_socket + j][k]);
	//        coreSums[i*session.core_gen_counter_num_used + k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
        coreSums[k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
      }
    }
  }

  uint64 *sums;
  sums = (uint64*)calloc(currentMachine.num_sockets*session.cbo_counter_num_used,sizeof(uint64));
  for (i = 0; i < currentMachine.num_sockets; i++) {
    for (j = 0; j < currentMachine.num_cbos; j++) {
      for (k = 0; k < session.cbo_counter_num_used; k++)
	//        sums[i*session.cbo_counter_num_used + k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
        sums[k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
    }
  }

  printf("%s,%s,%"PRIu64",%d,%d,%d,%f,",ts1,ts2,num_iters,M,N,K,seconds/(double)num_iters);
    for (j = 0; j < session.core_gen_counter_num_used; j++)
      //      printf("%"PRIu64",",after.generalCore[0][j]);
      printf("%f,",coreSums[j]/(double)num_iters);
    for (j = 0; j < session.cbo_counter_num_used; j++)
      printf("%f,",sums[j]/(double)num_iters);
    printf("\n");

  free(sums);
  free(coreSums);

  // Stop counters, reset PMU, close msr files
  cleanup(fd,&currentMachine,&session);

    
  free(A);
  free(B);
  free(C);
  
  return 0;
}
