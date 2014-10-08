#ifndef COUNTER_HOME_BREW
#define COUNTER_HOME_BREW

// commented out for hetero validation code...
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/msr.h>
#include <inttypes.h>

#include "registers.h"

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef int32_t int32;

#define ON 1
#define OFF 0

#define NEHALEM_EP 26
#define NEHALEM_EP_2 30
#define ATOM 28
#define ATOM_2 53
#define ATOM_3 54 // Centerton
#define CLARKDALE 37
#define WESTMERE_EP 44
#define NEHALEM_EX 46
#define WESTMERE_EX 47
#define SANDY_BRIDGE 42
#define JAKETOWN 45
#define IVY_BRIDGE 58
#define HASWELL 60
#define HASWELL_2 70

// USER DEFINED CONSTANTS...MODIFY HERE!!!

// Machine Constants
// IVB
/*
#define CPU_MODEL IVY_BRIDGE
#define NUM_PHYS_CORES_PER_SOCKET 4
#define NUM_LOGICAL_CORES_PER_PHYS_CORE 1
#define NUM_CORES_PER_SOCKET (NUM_PHYS_CORES_PER_SOCKET*NUM_LOGICAL_CORES_PER_PHYS_CORE)
#define NUM_SOCKETS 1
#define NUM_CORES (NUM_CORES_PER_SOCKET*NUM_SOCKETS)
#define CORE_GEN_COUNTER_MAX 4
#define CBO_COUNTER_NUM_MAX 2
#define NUM_IMC_CHANNELS 4
// ignoring the fixed imc channel that counts cycles for now...
#define IMC_COUNTER_NUM_MAX 4
*/

// NHM-EX

#define CPU_MODEL NEHALEM_EX
#define NUM_PHYS_CORES_PER_SOCKET 8
#define NUM_LOGICAL_CORES_PER_PHYS_CORE 1
#define NUM_CORES_PER_SOCKET (NUM_PHYS_CORES_PER_SOCKET*NUM_LOGICAL_CORES_PER_PHYS_CORE)
#define NUM_SOCKETS 4
#define NUM_CORES (NUM_CORES_PER_SOCKET*NUM_SOCKETS)
#define CORE_GEN_COUNTER_MAX 4
#define CBO_COUNTER_NUM_MAX 6
#define NUM_IMC_CHANNELS 4
// ignoring the fixed imc channel that counts cycles for now...
#define IMC_COUNTER_NUM_MAX 4


// JKT
/*
#define CPU_MODEL JAKETOWN
#define NUM_PHYS_CORES_PER_SOCKET 8
#define NUM_LOGICAL_CORES_PER_PHYS_CORE 1
#define NUM_CORES_PER_SOCKET (NUM_PHYS_CORES_PER_SOCKET*NUM_LOGICAL_CORES_PER_PHYS_CORE)
#define NUM_SOCKETS 2
#define NUM_CORES (NUM_CORES_PER_SOCKET*NUM_SOCKETS)
#define CORE_GEN_COUNTER_MAX 8
#define CBO_COUNTER_NUM_MAX 4
#define NUM_IMC_CHANNELS 4
// ignoring the fixed imc channel that counts cycles for now...
#define IMC_COUNTER_NUM_MAX 4
*/
// END USER DEFINED CONSTANTS

typedef struct {
  int32 cpu_model;
  int32 num_sockets;
  int32 num_cores;
  int32 num_cores_per_socket;
  int32 num_phys_cores_per_socket;

  // general purpose core counters
  int32 core_gen_counter_num_max;

  // uncore information
  int32 cbo_counter_num_max;
  int32 num_cbos;
  int32 imc_counter_num_max;
  int32 num_imc_channels;

} machineInformation;

typedef struct {
  int32 core_gen_counter_num_used;
  int32 core_event_numbers[CORE_GEN_COUNTER_MAX];
  int32 core_umasks[CORE_GEN_COUNTER_MAX];

  int32 cbo_counter_num_used;
  int32 cbo_filter;
  int32 cbo_event_numbers[CBO_COUNTER_NUM_MAX];
  int32 cbo_umasks[CBO_COUNTER_NUM_MAX];

  int32 imc_counter_num_used;
  int32 imc_event_numbers[IMC_COUNTER_NUM_MAX];
  int32 imc_umasks[IMC_COUNTER_NUM_MAX];

  
} counterSessionInfo;

typedef struct
{
  uint64 inst_retired_any_addr;
  uint64 cpu_clk_unhalted_thread_addr;
  uint64 cpu_clk_unhalted_ref_addr;
} fixedCounterData;

typedef struct
{
  uint64 generalCore[NUM_CORES][CORE_GEN_COUNTER_MAX];
  fixedCounterData fixedCore[NUM_CORES];
  uint64 cboUncore[NUM_PHYS_CORES_PER_SOCKET*NUM_SOCKETS][CBO_COUNTER_NUM_MAX];
} counterData;



typedef struct
{
  union
  {
    struct
    {
      // CTR0
      uint64 os0 : 1;
      uint64 usr0 : 1;
      uint64 any_thread0 : 1;
      uint64 enable_pmi0 : 1;
      // CTR1
      uint64 os1 : 1;
      uint64 usr1 : 1;
      uint64 any_thread1 : 1;
      uint64 enable_pmi1 : 1;
      // CTR2
      uint64 os2 : 1;
      uint64 usr2 : 1;
      uint64 any_thread2 : 1;
      uint64 enable_pmi2 : 1;
      
      uint64 reserved1 : 52;
    } fields;
    uint64 value;
  };
} FixedEventControlRegister;

typedef struct 
{
    union
    {
        struct
        {
            uint64 event_select : 8;
            uint64 umask : 8;
            uint64 usr : 1;
            uint64 os : 1;
            uint64 edge : 1;
            uint64 pin_control : 1;
            uint64 apic_int : 1;
            uint64 any_thread : 1;
            uint64 enable : 1;
            uint64 invert : 1;
            uint64 cmask : 8;
            uint64 in_tx : 1;
            uint64 in_txcp : 1;
            uint64 reservedX : 30;
        } fields;
        uint64 value;
    };
} EventSelectRegister;

// IVB desktop
typedef struct
{
  union
  {
    struct
    {
      uint64 pim_sel_core0 : 1;
      uint64 pim_sel_core1 : 1;
      uint64 pim_sel_core2 : 1;
      uint64 pim_sel_core3 : 1;
      uint64 res1 : 25;
      uint64 en : 1;
      uint64 wake_pmi : 1;
      uint64 frz : 1;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} IVBUncorePMONGlobalCtl;

typedef struct
{
  union
  {
    struct
    {
      uint64 ov_fixed : 1;
      uint64 ov_arb : 1;
      uint64 res1 : 1;
      uint64 ov_cbo : 1;
      uint64 res2 : 60;
    } fields;
    uint64 value;
  };
} IVBUncorePMONGlobalStatus;

typedef struct
{
  union
  {
    struct
    {
      uint64 cbo_status : 4;
      uint64 res2 : 60;
    } fields;
    uint64 value;
  };
} IVBUncoreCboConfig;

typedef struct
{
  union
  {
    struct
    {
      uint64 event_count: 44;
      uint64 reserved : 20;
    } fields;
    uint64 value;
  };
} IVBCboCounter;

typedef struct
{
  union
  {
    struct
    {
      uint64 num_cbo: 4;
      uint64 reserved : 60;
    } fields;
    uint64 value;
  };
} IVBCboConfig;

typedef struct
{
  union
  {
    struct
    {
      uint64 event_select: 8;
      uint64 umask: 8;
      uint64 res1: 2;
      uint64 edge_det: 1;
      uint64 res2: 1;
      uint64 ov_en: 1;
      uint64 res3 : 1;
      uint64 en : 1;
      uint64 inv : 1;
      uint64 cmask : 5;
      uint64 reserved : 35;
    } fields;
    uint64 value;
  };
} IVBCboPerfEvtSel;

// for NHM-EX
typedef struct
{
  union
  {
    struct
    {
      uint64 en : 1;
      uint64 pmi_core_sel : 8;
      uint64 ig : 19;
      uint64 en_all : 1;
      uint64 rst_all : 1;
      uint64 ig2 : 1;
      uint64 frz_all : 1;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXUncorePMONGlobalCtl;

typedef struct
{
  union
  {
    struct
    {
      uint64 ov_u : 1;
      uint64 ov_w : 1;
      uint64 ov_s1 : 1;
      uint64 ov_s0 : 1;
      uint64 ig : 26;
      uint64 pmi : 1;
      uint64 cond : 1;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXUncorePMONGlobalStatus;

typedef struct
{
  union
  {
    struct
    {
      uint64 clr_ov_u : 1;
      uint64 clr_ov_w : 1;
      uint64 clr_ov_s1 : 1;
      uint64 clr_ov_s0 : 1;
      uint64 ig : 26;
      uint64 clr_pmi : 1;
      uint64 clr_cond : 1;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXUncorePMONGlobalOvfCtl;

// is this really a 32-bit calender??? 
typedef struct 
{
    union
    {
        struct
        {
            uint64 event_select : 8;
            uint64 umask : 8;
            uint64 reserved1 : 1;
            uint64 occ_ctr_rst : 1;
            uint64 edge : 1;
            uint64 reserved2 : 1;
            uint64 enable_pmi : 1;
            uint64 reserved3 : 1;
            uint64 enable : 1;
            uint64 invert : 1;
            uint64 cmask : 8;
            uint64 reservedx : 32;
        } fields;
        uint64 value;
    };
} NHMEXUncoreEventSelectRegister;

typedef struct
{
  union
  {
    struct
    {
      uint64 ctr_en : 6;
      uint64 reserved : 26;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXCboPMONGlobalCtl;

typedef struct
{
  union
  {
    struct
    {
      uint64 ov : 6;
      uint64 reserved : 26;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXCboPMONGlobalStatus;

typedef struct
{
  union
  {
    struct
    {
      uint64 clr_ov : 6;
      uint64 reserved : 26;
      uint64 resvx : 32;
    } fields;
    uint64 value;
  };
} NHMEXCboPMONGlobalOverflowCtl;

typedef struct
{
  union
  {
    struct
    {
      uint64 ev_sel : 8;
      uint64 umask : 8;
      uint64 ig : 2;
      uint64 edge_detect : 1;
      uint64 ig2 : 1;
      uint64 pmi_en : 1;
      uint64 ig3 : 1;
      uint64 en : 1;
      uint64 invert : 1;
      uint64 threshold : 8;
      uint64 rsv2 : 18; // irritating gap in specification...
      uint64 ig4 : 11;
      uint64 rsv : 2;
      uint64 ig5 : 1;
    } fields;
    uint64 value;
  };
} NHMEXCboPMONEventSel;

typedef struct
{
  union
  {
    struct
    {
      uint64 event_count: 48;
      uint64 reserved : 16;
    } fields;
    uint64 value;
  };
} NHMEXCboPMONCounter;

// For Jaketown
typedef struct 
{
    union
    {
        struct
        {
	  uint64 rst_ctrl : 1;
	  uint64 rst_ctrs : 1;
	  uint64 resv0 : 6;
	  uint64 frz : 1;
	  uint64 rsv1 : 7;
	  uint64 frz_en : 1;
	  uint64 resv2 : 1;
	  uint64 resv3 : 14;
	  uint64 resvx : 32;
        } fields;
        uint64 value;
    };
} JKTCBoxCTLRegister;

typedef struct 
{
    union
    {
        struct
        {
	  uint64 ev_sel : 8;
	  uint64 umask : 8;
	  uint64 resv0 : 1;
	  uint64 rst : 1;
	  uint64 edge_det : 1;
	  uint64 tid_en : 1;
	  uint64 resv1 : 2;
	  uint64 en : 1;
	  uint64 invert : 1;
	  uint64 thresh : 8;
	  uint64 resvx : 32;
        } fields;
        uint64 value;
    };
} JKTCBoxPMONCTLRegister;

typedef struct 
{
    union
    {
        struct
        {
	  uint64 tid : 5;
	  uint64 resv0 : 3;
	  uint64 resv1 : 2;
	  uint64 nid : 8;
	  uint64 state : 5;
	  uint64 opc : 9;
	  uint64 resvx : 32;
        } fields;
        uint64 value;
    };
} JKTCBoxPMONFilterRegister;

typedef struct 
{
    union
    {
        struct
        {
	  uint64 event_count : 44;
	  uint64 resv0 : 20;
        } fields;
        uint64 value;
    };
} JKTCBoxPMONCTR;

void open_msr_file(int cpu, int* fd);
void close_msr_file(int *fd);
void read_msr(int fd, uint64 msr, uint64 *output);
void write_msr(int fd, uint64 msr, uint64 *output);
int32 PMUinUse(int *fd, machineInformation *machine);
void toggleJKTCboCounters(int currentCoreFD, int num_cbos, counterSessionInfo *session, int state);
void toggleJKTUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state);
void toggleUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state);
void stopCounters(int core, int currentCoreFD, machineInformation *machine, counterSessionInfo *session);
void startCounters(int core, int currentCoreFD, machineInformation *machine, counterSessionInfo *session);
void programCoreFixedCounters(int currentCoreFD);
void programGeneralPurposeRegisters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session);
void readJKTCboCounters(int socket, int currentCoreFD, int num_cbos, int cbo_counters_used, counterData *data);
void readJKTUncoreCounters(int socket, int currentCoreFD, machineInformation *machine, counterSessionInfo *session, counterData *data);
void readUncoreCounters(int core, int currentCoreFD,machineInformation *machine, counterSessionInfo *session, counterData *data);
void readCounters(int *fd, machineInformation *machine, counterSessionInfo *session, counterData *data);
void diffFixedCounterData(machineInformation * machine, counterData *final, counterData *before, counterData * after);
void diffGeneralCounterData(machineInformation *machine, counterSessionInfo *session, counterData *final, counterData *before, counterData *after);
void programJKTCbo(int currentCoreFD, int num_cbos, counterSessionInfo *session);
void programJKTUncore(int currentCoreFD, machineInformation *machine, counterSessionInfo *session);
void programUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session);
void cleanup(int *fd, machineInformation *machine, counterSessionInfo *session);
void diffJKTCboCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after);
void diffJKTUncoreCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after);
void diffUncoreCounterData(machineInformation *machine, counterSessionInfo *session, counterData* final, counterData *before, counterData *after);
void diffCounterData(machineInformation *machine, counterSessionInfo *session, counterData* final, counterData *before, counterData *after);

#endif
















