#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <immintrin.h>
#include <assert.h>


// CUDA runtime
#include <cuda_runtime.h>
#include <cublas_v2.h>

#include "CounterHomeBrew.h"
#include <omp.h>
#include <mkl.h>

#define BLASFUNC cblas_sgemm
#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif
#ifndef max
#define max(a,b) ((a > b) ? a : b)
#endif

int block_size;

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

// Allocates a matrix with random float entries.
void randomInit(float *data, int size)
{
  for (int i = 0; i < size; ++i)
    data[i] = rand() / (float)RAND_MAX;
}
void testInit(float *data, int size)
{
  for (int i = 0; i < size; ++i)
    data[i] = (float)i;
}

void printMatrix(float *data,int size) {
  for (int i = 0; i < size; i++)
    printf("%f ",data[i]);
}

void initializeCUDA() {
  cudaError_t error;
  int devID = 0;

  error = cudaSetDevice(devID); if (error != cudaSuccess){printf("cudaSetDevice returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
  error = cudaGetDevice(&devID); if (error != cudaSuccess){printf("cudaGetDevice returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}

  //  printf("Device ID is %d\n",devID);

  cudaDeviceProp deviceProp;
  error = cudaGetDeviceProperties(&deviceProp,devID); if (error != cudaSuccess){printf("cudaGetDeviceProperties returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}

  //  printf("GPU Device %d: \"%s\" with compute capability %d.%d\n\n", devID, deviceProp.name, deviceProp.major, deviceProp.minor);

  // use larger block size for Fermi and above
  block_size = (deviceProp.major < 2) ? 16 : 32;
}

void GPUsgemm(int gpuInner, int Md,int Nd,int Kd,float* Adevice,float *Bdevice,float *Cdevice,float *Ahost,float *Bhost,float *Chost, cudaStream_t *stream) {
  cudaError_t error;
  int memSizeA = sizeof(float)*Md*Kd;
  int memSizeB = sizeof(float)*Kd*Nd;
  int memSizeC = sizeof(float)*Md*Nd;

  error = cudaMemcpyAsync(Adevice,Ahost,memSizeA,cudaMemcpyHostToDevice,*stream); if (error != cudaSuccess){printf("cudaMemcpy A returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
  error = cudaMemcpyAsync(Bdevice,Bhost,memSizeB,cudaMemcpyHostToDevice,*stream); if (error != cudaSuccess){printf("cudaMemcpy B returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}

  // setup execution parameters
  dim3 threads(block_size,block_size);
  dim3 grid(Nd/threads.x,Md/threads.y);

  // inside CUBLAS
  cublasHandle_t handle;
  cublasStatus_t ret;
  ret = cublasCreate(&handle); if (ret != CUBLAS_STATUS_SUCCESS){printf("cublasCreate returned error code %d, line(%d)\n", ret, __LINE__);exit(EXIT_FAILURE);}
  const float alpha = 1.0f;
  const float beta  = 0.0f;
  cublasSetStream(handle,*stream);
  for (int i = 0; i < gpuInner; i++) {
    ret = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, Nd, Md, Kd, &alpha, Bdevice, Nd, Adevice, Kd, &beta, Cdevice, Nd);
    if (ret != CUBLAS_STATUS_SUCCESS) {
      printf("cublasSgemm returned error code %d, line(%d)\n", ret, __LINE__);
      exit(EXIT_FAILURE);
    }
  }
  // done CUBLAS

  // copy result back to host
  error = cudaMemcpyAsync(Chost,Cdevice,memSizeC,cudaMemcpyDeviceToHost,*stream);
  //  printf("GPU Iter queued\n");
}

int main(int argc, char *argv[]) {
  int i,j,k;
  machineInformation currentMachine;
  counterSessionInfo session;

  initializeCUDA();

  // Set machine information from CounterHomeBrew.h
  currentMachine.cpu_model = CPU_MODEL;
  currentMachine.num_sockets = NUM_SOCKETS;
  currentMachine.num_phys_cores_per_socket = NUM_PHYS_CORES_PER_SOCKET;
  currentMachine.num_cores_per_socket = NUM_CORES_PER_SOCKET;
  currentMachine.num_cores = NUM_CORES;
  currentMachine.num_cbos = NUM_PHYS_CORES_PER_SOCKET; // should multiply by NUM_SOCKETS???
  currentMachine.core_gen_counter_num_max = CORE_GEN_COUNTER_MAX;
  currentMachine.cbo_counter_num_max = CBO_COUNTER_NUM_MAX;

  // Set session events, umasks and counters used
  //  int32 core_event_numbers[] = {FP_COMP_OPS_EXE_EVTNR,SIMD_FP_256_EVTNR,0x51,0xF1,0x80};
  // int32 core_umasks[] = {FP_COMP_OPS_EXE_SCALAR_DOUBLE_UMASK,SIMD_FP_256_PACKED_DOUBLE_UMASK,0x01, 0x07,0x01};

  session.core_gen_counter_num_used = 5;
  int32 core_event_numbers[] = {0x10,0x10,0x11,0x51,0xF1};
  int32 core_umasks[] = {0x20,0x40,0x01,0x01, 0x07};

  session.cbo_counter_num_used = 1;
  int32 cbo_event_numbers[] = {0x37};
  int32 cbo_umasks[] = {0xf};
  session.cbo_filter = 0x1f;

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
  

  // M,N,K are multiples of the block size....
  int gpuOuter = atoi(argv[1]);
  int gpuInner = atoi(argv[2]);
  int cpuInner = atoi(argv[3]);
  double minRuntime = atoi(argv[4]);
  int Md = atoi(argv[5])*block_size;
  int Nd = atoi(argv[6])*block_size;
  int Kd = atoi(argv[7])*block_size;
  int Mh = atoi(argv[8]);
  int Nh = atoi(argv[9]);
  int Kh = atoi(argv[10]);

  char *ts1,*ts2,*ts3,*ts4;
  char *ts5,*ts6,*ts7,*ts8;
  double fineTimeStamps[8];
  double gTime = 0.0;
  double cTime = 0.0;
  double seconds = 0.0;
  int num_iters;

  uint64 *coreSums;
  coreSums = (uint64*)calloc(currentMachine.num_sockets*session.core_gen_counter_num_used,sizeof(uint64));

  uint64 *sums;
  sums = (uint64*)calloc(currentMachine.num_sockets*session.cbo_counter_num_used,sizeof(uint64));

  // calculate how many iterations we need on the CPU to get the desired min runtime
  float *Atmp = NULL;
  float *Btmp = NULL;
  float *Ctmp = NULL;
  Atmp = (float*) malloc( Mh * Kh * sizeof(float) );
  Btmp = (float*) malloc( Kh * Nh * sizeof(float) );
  Ctmp = (float*) malloc( Mh * Nh * sizeof(float) );
  randomInit(Atmp,Mh*Kh);
  randomInit(Btmp,Kh*Nh);

  for (num_iters = cpuInner; seconds < minRuntime; num_iters *=2) {
    seconds = 0.0;
    seconds = read_timer();
    for (i =0; i < num_iters; i++)
      BLASFUNC( CblasColMajor,CblasNoTrans, CblasNoTrans,Mh,Nh,Kh, 1, Atmp,Mh, Btmp,Kh, 0, Ctmp,Mh );
    seconds = read_timer()-seconds;
  }
  num_iters /= 2;

  free(Atmp);
  free(Btmp);
  free(Ctmp);

  int readyThreads = 0;
  #pragma omp parallel
  {
    int threadNum = omp_get_thread_num();
    int numThreads = omp_get_num_threads();
    assert(numThreads==2);
    if (threadNum == 0) {
      cudaError_t error;
      int memSizeA = sizeof(float)*Md*Kd;
      int memSizeB = sizeof(float)*Kd*Nd;
      int memSizeC = sizeof(float)*Md*Nd;
      
      float *Ahost,*Bhost,*Chost;
      // use pinned memory on the host for BW and asynch memory transfers..
      int flags = cudaHostAllocDefault;
      ts5 = getTimeStamp();
      fineTimeStamps[0] = read_timer();
      error = cudaHostAlloc((void**)&Ahost,memSizeA,flags);if (error != cudaSuccess){printf("cudaHostMalloc Ahost returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      error = cudaHostAlloc((void**)&Bhost,memSizeB,flags);if (error != cudaSuccess){printf("cudaHostMalloc Bhost returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      error = cudaHostAlloc((void**)&Chost,memSizeC,flags);if (error != cudaSuccess){printf("cudaHostMalloc Chost returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      // set local arrays
      randomInit(Ahost,Md*Kd);
      randomInit(Bhost,Kd*Nd);

      // allocate device memory
      float *Adevice,*Bdevice,*Cdevice;
      error = cudaMalloc((void**)&Adevice,memSizeA); if (error != cudaSuccess){printf("cudaMalloc Adevice returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      error = cudaMalloc((void**)&Bdevice,memSizeB); if (error != cudaSuccess){printf("cudaMalloc Bdevice returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      error = cudaMalloc((void**)&Cdevice,memSizeC); if (error != cudaSuccess){printf("cudaMalloc Cdevice returned error code %d, line(%d)\n", error, __LINE__);exit(EXIT_FAILURE);}
      fineTimeStamps[1] = read_timer();
      ts6 = getTimeStamp();
      //      fprintf(stderr,"Incrementing ready GPU\n");
#pragma omp critical
      {
	readyThreads += 1;
      }
      //     fprintf(stderr,"Incremented ready GPU\n");
      while (readyThreads < 2){sleep(1);fprintf(stderr,"Thread 0: %d\n",readyThreads);};
      //#pragma omp single 
      //{
      cudaStream_t stream1;
      cudaStreamCreate ( &stream1) ;
      ts3 = getTimeStamp();
      fineTimeStamps[2] = read_timer();
      gTime = read_timer();
      for (int i = 0; i < gpuOuter; i++) 
	GPUsgemm(gpuInner,Md,Nd,Kd,Adevice,Bdevice,Cdevice,Ahost,Bhost,Chost,&stream1);
      cudaStreamSynchronize(stream1);
      gTime = read_timer() - gTime;
      fineTimeStamps[3] = read_timer();
      ts4 = getTimeStamp();
      cudaFreeHost(Ahost);
      cudaFreeHost(Bhost);
      cudaFreeHost(Chost);

    } else {
      float *A = NULL;
      float *B = NULL;
      float *C = NULL;
      ts7 = getTimeStamp();
      fineTimeStamps[4] = read_timer();
      A = (float*) malloc( Mh * Kh * sizeof(float) );
      B = (float*) malloc( Kh * Nh * sizeof(float) );
      C = (float*) malloc( Mh * Nh * sizeof(float) );
      randomInit(A,Mh*Kh);
      randomInit(B,Kh*Nh);
      fineTimeStamps[5] = read_timer();
      ts8 = getTimeStamp();
      //    fprintf(stderr,"Incrementing ready CPU\n");
#pragma omp critical
      {
	readyThreads += 1;
      }
      //   fprintf(stderr,"Incremented ready CPU\n");
      while (readyThreads < 2){sleep(1);fprintf(stderr,"Thread 1: %d\n",readyThreads);};
                  
      // open the msr files for each core on the machine
      for (i = 0; i < currentMachine.num_cores; i++)
	open_msr_file(i,&fd[i]);
      
      
      int socketsProgrammed = 0;
      for (i = 0; i < currentMachine.num_cores; i++) {
	int currentCoreFD = fd[i];
	
	stopCounters(i, currentCoreFD, &currentMachine, &session);
	programCoreFixedCounters(currentCoreFD);    
	programGeneralPurposeRegisters(currentCoreFD, &currentMachine, &session);
	
	/* Program the Uncore as desired...*/
	// Only program the first physical core on each socket. 
	// NOTE: Some assumptions about topology here...check /proc/cpuinfo to confirm.
	if (i % currentMachine.num_phys_cores_per_socket == 0 && socketsProgrammed < currentMachine.num_sockets) {
	  programUncoreCounters( currentCoreFD, &currentMachine, &session);
	  socketsProgrammed++;
	}
      }
      
      //      sleep(5);
      seconds = 0.0;
      // start the programmed counters...
      for (i = 0; i < currentMachine.num_cores; i++)
	startCounters( i, fd[i], &currentMachine, &session);
	
      /* READ COUNTERS BEFORE STUFF */
      readCounters(fd,&currentMachine,&session, &before);
      ts1 = getTimeStamp();
      fineTimeStamps[6] = read_timer();
      seconds = read_timer();
	
	/* DO STUFF */    
      for (i =0; i < num_iters; i++)
	BLASFUNC( CblasColMajor,CblasNoTrans, CblasNoTrans,Mh,Nh,Kh, 1, A,Mh, B,Kh, 0, C,Mh );
	/* END DOING STUFF */
	
      seconds = read_timer()-seconds;
      fineTimeStamps[7] = read_timer();
      ts2 = getTimeStamp();
	
	/* READ COUNTERS AFTER STUFF */    
      for (i = 0; i < currentMachine.num_cores; i++)
	stopCounters(i,fd[i],&currentMachine, &session);
      
	//  printf("num_iters = %"PRIu64", runtime is %g\n",num_iters,seconds);
            
      readCounters(fd,&currentMachine,&session,&after);
      diffCounterData(&currentMachine, &session, &after, &before, &after);
      
      for (i = 0; i < currentMachine.num_sockets; i++) {
	//    printf("Socket %d\n",i);
	for (j = 0; j < currentMachine.num_cores_per_socket; j++) {
	  //   printf("%d,",j);
	  for (k = 0; k < session.core_gen_counter_num_used; k++){
	    //	printf("%"PRIu64",",after.generalCore[i*currentMachine.num_cores_per_socket + j][k]);
	    // bug in the indexing of the core sums???
	    //        coreSums[i*session.core_gen_counter_num_used + k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
	    coreSums[k] += after.generalCore[i*currentMachine.num_cores_per_socket + j][k];
	  }
	  //	printf("\n");
	}
      }
      
      for (i = 0; i < currentMachine.num_sockets; i++) {
	//	printf("%d,",i);
	for (j = 0; j < currentMachine.num_cbos; j++) {
	  //	  printf("%d,",j);
	  for (k = 0; k < session.cbo_counter_num_used; k++) {
	    //	    printf("%llu,",after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k]);
	    // bug in the indexing of the core sums???
	    //        sums[i*session.cbo_counter_num_used + k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
	    sums[k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
	  }
	}
      }
      //      printf("\n");
            
      // Stop counters, reset PMU, close msr files
      cleanup(fd,&currentMachine,&session);
      
      
      free(A);
      free(B);
      free(C);
    }
  } // end parallel region

  //  printf("%d,%s,%s,%d,%d,%d,%d,%d,%g,%g\n",threadNum,ts3,ts4,Md,Nd,Kd,gpuOuter,gpuInner,gTime,(double)(gpuOuter*(Md*Kd+Nd*Kd+Md*Nd))/16.0);
  //  printf("%d,%s,%s,%d,%d,%d,%d,%f,",threadNum,ts1,ts2,num_iters,Mh,Nh,Kh,seconds/(double)num_iters);

  printf("%s,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,",ts7,ts8,ts1,ts2,ts5,ts6,ts3,ts4,Mh,Nh,Kh,Md/block_size,Nd/block_size,Kd/block_size,num_iters,gpuOuter,gpuInner,seconds,gTime,(float)(gpuOuter*(Md*Kd+Nd*Kd+Md*Nd))/16.0);
  for (int i = 0; i < 8; i++)
    printf("%f,",fineTimeStamps[i]);
  for (j = 0; j < session.core_gen_counter_num_used; j++)
    //    printf("%f,",coreSums[j]/(double)num_iters);
    printf("%llu,",coreSums[j]);
  for (j = 0; j < session.cbo_counter_num_used; j++)
    if (j == session.cbo_counter_num_used-1)
      //      printf("%f",sums[j]/(double)num_iters);
      printf("%llu",sums[j]);
    else
      //      printf("%f,",sums[j]/(double)num_iters);
      printf("%llu,",sums[j]);
  printf("\n");
  
  free(sums);
  free(coreSums);
  
  return 0;
}

/*
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
*/
