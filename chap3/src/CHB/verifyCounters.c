// Sequential code to execute memory microbenchmarks to evaluate counters for accuracy. 
#include "CounterHomeBrew.h"
#include "loops.h"
#include "pthread.h"
/*
  switch(test){
  case 0:
    sumloop(N,1);
    break;
  case 1:
    sumloop(N,10);
    break;
  case 2:
    copyloop(N,10);
    break;
  case 3:
    transformloop(N,1);
    break;
  case 4:
    transformloop(N,3);
    break;
  case 5:
    copyloopStreaming(N,10);
    break;
    
  }
*/
int main(int argc, char *argv[]) {
  int i,j,k;
  machineInformation currentMachine;
  counterSessionInfo session;

  // Set machine information from CounterHomeBrew.h
  currentMachine.cpu_model = CPU_MODEL;
  currentMachine.num_sockets = NUM_SOCKETS;
  currentMachine.num_phys_cores_per_socket = NUM_PHYS_CORES_PER_SOCKET;
  currentMachine.num_cores_per_socket = NUM_CORES_PER_SOCKET;
  currentMachine.num_cores = NUM_CORES;
  currentMachine.num_cbos = NUM_PHYS_CORES_PER_SOCKET;
  currentMachine.core_gen_counter_num_max = CORE_GEN_COUNTER_MAX;
  currentMachine.cbo_counter_num_max = CBO_COUNTER_NUM_MAX;

  // Set session events, umasks and counters used
  int32 core_event_numbers[] = {FP_COMP_OPS_EXE_EVTNR,0x51,0xF1};
  int32 core_umasks[] = {FP_COMP_OPS_EXE_SCALAR_DOUBLE_UMASK,0x01, 0x07};
  // cbo events
  int32 cbo_event_numbers[] = {0x37};
  int32 cbo_umasks[] = {0xf};
  session.core_gen_counter_num_used = 3;
  session.cbo_counter_num_used = 1;
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
  cpu_set_t old_affinity;
  cpu_set_t new_affinity;
  pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &old_affinity);
  CPU_ZERO(&new_affinity);
  CPU_SET(0, &new_affinity);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &new_affinity);

  // Arrays to hold counter data...
  counterData before;
  counterData after;

  int test = atoi(argv[1]);
  int N = atoi(argv[2]);
  // some data for doing a naive matmul to test flop counting...
  initloop(N);

 // open the msr files for each core on the machine
  for (i = 0; i < currentMachine.num_cores; i++)
    open_msr_file(i,&fd[i]);

  /* Checking to see if core PMU isn't in use */

  /*
  if (PMUinUse(fd,&currentMachine)) {
    printf("PMU appears to be in use...\n");
    return -1;
  }
  */
  flush_cache(N);

  // PMU is not in use...program the counters!!!
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
    if (i % currentMachine.num_phys_cores_per_socket == 0 && socketsProgrammed < currentMachine.num_sockets) {
      programUncoreCounters( currentCoreFD, &currentMachine, &session);
      socketsProgrammed++;
    }

    /* set global control register to active counters */
    startCounters( i, currentCoreFD, &currentMachine, &session);

  }

  /* READ COUNTERS BEFORE STUFF */
  readCounters(fd,&currentMachine,&session, &before);

  /* DO STUFF */
  
  
  switch(test){
  case 0:
    sumloop(N,1);
    break;
  case 1:
    sumloop(N,10);
    break;
  case 2:
    copyloop(N,10);
    break;
  case 3:
    transformloop(N,1);
    break;
  case 4:
    transformloop(N,3);
    break;
  case 5:
    copyloopStreaming(N,10);
    break;
    
  }
  
  /* END DOING STUFF */
  
  /* READ COUNTERS AFTER STUFF */

  for (i = 0; i < currentMachine.num_cores; i++)
    stopCounters(i,fd[i],&currentMachine, &session);

  readCounters(fd,&currentMachine,&session,&after);
  diffCounterData(&currentMachine, &session, &after, &before, &after);

  uint64 *coreSums;
  coreSums = (uint64*)calloc(currentMachine.num_sockets*session.core_gen_counter_num_used,sizeof(uint64));
  for (i = 0; i < currentMachine.num_sockets; i++) {
    for (j = 0; j < currentMachine.num_cores_per_socket; j++) {
      for (k = 0; k < session.core_gen_counter_num_used; k++)
        coreSums[i*session.core_gen_counter_num_used + k] += after.generalCore[i*currentMachine.num_phys_cores_per_socket + j][k];
    }
  }

  uint64 *sums;
  sums = (uint64*)calloc(currentMachine.num_sockets*session.cbo_counter_num_used,sizeof(uint64));
  for (i = 0; i < currentMachine.num_sockets; i++) {
    for (j = 0; j < currentMachine.num_cbos; j++) {
      for (k = 0; k < session.cbo_counter_num_used; k++)
        sums[i*session.cbo_counter_num_used + k] += after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k];
    }
  }

    // Print the counter information for each core

  // only print data from first socket and core
  printf("%d,",N);
    for (j = 0; j < session.core_gen_counter_num_used; j++)
      printf("%"PRIu64",",after.generalCore[0][j]);
    //  printf("%f,",coreSums[j]);
    for (j = 0; j < session.cbo_counter_num_used; j++)
      printf("%"PRIu64"",sums[j]);
    printf("\n");
      //   printf("\t\tGeneral Purpose Counter %d: %"PRIu64"\n",j,after.generalCore[i][j]);
	//  }
    /*
  printf("\n\n");

  for (i = 0; i < currentMachine.num_sockets; i++) {
    printf("Socket %d Uncore Counters: \n",i);
    printf("\tCbo Counters:\n");
    for (j = 0; j < currentMachine.num_cbos; j++) {
      printf("\t\tCbo %d:\n",j);
      for (k = 0; k < session.cbo_counter_num_used; k++)
	printf("\t\tCounter %d: %"PRIu64"\n",k,after.cboUncore[i*currentMachine.num_phys_cores_per_socket + j][k]);
    }
      for (k = 0; k < session.cbo_counter_num_used; k++)
	printf("\t\tSum Counter %d: %"PRIu64"\n",k,sums[i*session.cbo_counter_num_used+k]);
  }
    */					
  free(sums);
  free(coreSums);
  // Stop counters, reset PMU, close msr files
  cleanup(fd,&currentMachine,&session);


  return 0;
}
