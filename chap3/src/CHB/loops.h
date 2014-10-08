#include <inttypes.h>           /* for PRIu64 definition */
#include <stdint.h>             /* for uint64_t and PRIu64 */
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <immintrin.h>

#define CACHELINE 64  // bytes

typedef uint64_t E;

typedef void (*testFunction)(int,int);

void * mallocAligned(uint64_t  size, uint64_t align) {
  uintptr_t mask = ~((uintptr_t)(align-1));
  void *mem = malloc(size+align-1+sizeof(uintptr_t));
  void *ptr = (void*)( (uintptr_t)((char*)mem+align-1) & mask);
  assert((align & (align-1)) == 0);
  ((uintptr_t*)ptr)[-1] = (uintptr_t)mem;
  return ptr;
}

void freeAligned(void *alignedBuffer) {
  free( (void*)((uintptr_t*)alignedBuffer)[-1]);
}

void flush_cache (int len) {
  int i,j;
  //  E *flush = (E*)malloc(len*sizeof(E)); E sum = 0;
  E *flush = (E*)mallocAligned(len*sizeof(E),32); E sum = 0;
  for (i=0; i<len; ++i)
    flush[i]=3*i;
  for (j=1; j<3; ++j)
    for (i=0; i<len; ++i)
      sum += flush[i];
  //free(flush);
  freeAligned(flush);
  if (sum==0) {printf("Something\n");}
}

E* A;
E* B;
struct perf_data* pd; 

void initloop(int n) {
  register int i;
  //A = (E*)malloc(n*sizeof(E));
  //B = (E*)malloc(n*sizeof(E));
  A = (E*)mallocAligned(n*sizeof(E),32);
  B = (E*)mallocAligned(n*sizeof(E),32);
  for (i=0; i<n; ++i) {
    A[i] = i;
    B[i] = 2*i;
  }
}

void copyloop(int n, int repeats) {
  register int j;
  register int i;
  for (j=0; j<repeats; ++j)
    for (i=0; i<n; ++i)
      B[i] = 1+A[i];
}

void copyloopStreaming(int n, int repeats) {
  register int j;
  register int i;
  // __m256d one = _mm256_set1_pd(1.0);
  for (j=0; j<repeats; ++j)
    //    for (i = 0; i < n/4*4; i+=4) {
    //   __m256d tmp = _mm256_add_pd(_mm256_loadu_pd(A+i),one);
    //  _mm256_stream_pd(B+i,tmp);
    //  }
        for (i=0; i<n; ++i)
	  B[i] = 1+A[i];
}

void copyloopptr(int n, int repeats) {
  register E * Ap = A;
  register E * Bp = B;
  register int j;
  register int i;
  
  for (j=0; j<repeats; ++j)
    for (i=0; i<n; ++i)
      *Bp++ = 1+*Ap++;
}

void transformloop(int n, int repeats) {
  register int j;
  register int i;

  for (j=0; j<repeats; ++j) 
    for (i=0; i<n; ++i)
      A[i] = 1+A[i];
  if (A[n/repeats]==0) printf("Something\n");
}

void setA(int n) {
  register int i;

  for (i=0; i<n; ++i)
    A[i] =  i;
}
void nothing(int n) {
    register E x = 0;
    register int i;

    for (i=0; i<n; ++i) { x+=A[0];}
}

void almostnothing(int n) {
    register E x = 0;
    register int i;

    for (i=0; i<n; ++i) { x+=A[0];}
    if(x == 1) printf("One!\n"); // Just to make sure compiler does not optimize the loop away
}

void sumloop(int n, int repeats) {
    register E x = 0;

    register int i;
    register int j;
    for (j=0; j<repeats; ++j) 
      for (i=0; i<n; ++i)
	x += A[i];
    if(x == 0) printf("Zero!\n") ; // Just to make sure compiler does not optimize the loop away
}

void sumloopptr(int n, int repeats) {
    register E x = 0;
    register E * Ap = A;

    register int i;
    register int j;
    for (j=0; j<repeats; ++j) 
      for (i=0; i<n; ++i)
        x += *Ap++;
    if(x == 0) printf("Zero!\n") ; // Just to make sure compiler does not optimize the loop away
}


void sumloopptr_unroll8(int n, int repeats) {
    register E x = 0;
    register E * Ap = A;
    register int i;
    register int j;

    for (j=0; j<repeats; ++j)
      for (i=0; i<n; i+=8) {
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
        x += *Ap++;
    }
    if(x == 0) printf("Zero!\n") ; // Just to make sure compiler does not optimize the loop away
}


void jumpysumloop(int n, int repeats) {
    register E x = 0;
    register int i;
    register int j;

    for (j=0; j<repeats; ++j) 
      for (i=0; i<n; ++i)
        x += A[(i*33397)%n];
    if(x == 0) printf("Zero!\n") ; // Just to make sure compiler does not optimize the loop away
}

