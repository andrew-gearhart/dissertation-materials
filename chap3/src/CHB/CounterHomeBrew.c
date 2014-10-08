#include "CounterHomeBrew.h"

void open_msr_file(int cpu, int* fd) {
  char msr_file_name[64];
  
  sprintf(msr_file_name,"/dev/cpu/%d/msr",cpu);
  *fd = open(msr_file_name,O_RDWR);
  if (*fd < 0) {
    if (errno == ENXIO) {
      fprintf(stderr,"No CPU %d\n",cpu);
      exit(2);
    } else if (errno == EIO) {
      fprintf(stderr, "CPU %d doesn't support MSRs...have you loaded the kernel module \"msr\"?\n",cpu);
      exit(3);
    } else {
      perror("open");
      exit(127);
    }
  }
}

void close_msr_file(int *fd) {
  close(*fd);
}

// dont try to write a function that reads only 32 bits...as the RDMSR instruction always returns 64 bits.
// if the MSR is only 32-bits, use a 64-bit data structure and reserve the high-order bits
void read_msr(int fd, uint64 msr, uint64 *output) {  
  int32 ret = 0;
  ret = pread(fd, (void*)output, sizeof(uint64), msr) ;
  if (ret != sizeof(uint64)) {
    if (errno == EIO) {
      fprintf(stderr, "Cannot read "
	      "MSR 0x%08"PRIx64"\n",
	      msr);
      exit(4);
    } else {
      perror("pread");
      exit(127);
    }
  }
  
}

void write_msr(int fd, uint64 msr, uint64 *output) {
  int32 ret = 0;
  ret = pwrite(fd, (const void*)output, sizeof(uint64), msr);
 if (ret != sizeof(uint64)) {
    if (errno == EIO) {
      fprintf(stderr, "Cannot write "
	      "MSR 0x%08"PRIx64"\n",
	      msr);
      exit(4);
    } else {
      perror("pwrite");
      exit(127);
    }
  }
  
}


// from Intel PMU code and ported to C...
int32 PMUinUse(int *fd, machineInformation *machine) {
  uint32 i,j;
  for (i = 0; i < machine->num_cores; i++) {
    uint64 value;
    int currentCoreFD = fd[i];

    read_msr( currentCoreFD, IA32_CR_PERF_GLOBAL_CTRL, &value );    
    EventSelectRegister event_select_reg;
    event_select_reg.value = 0xFFFFFFFFFFFFFFFF;
    
    for (j = 0; j < machine->core_gen_counter_num_max; ++j)
      {
	read_msr(currentCoreFD, (0x186) + j, &event_select_reg.value);
	
	if (event_select_reg.fields.event_select != 0 || event_select_reg.fields.apic_int != 0)
	  {
	    printf( "WARNING: Core %u IA32_PERFEVTSEL0_ADDR are not zeroed %"PRIx64"\n",i,event_select_reg.value);
	    return 1;
	  }
      }
    
    FixedEventControlRegister ctrl_reg;
    ctrl_reg.value = 0xffffffffffffffff;
    
    read_msr(currentCoreFD, (0x38D), &ctrl_reg.value);
        
    if(ctrl_reg.fields.enable_pmi0 || ctrl_reg.fields.enable_pmi1 || ctrl_reg.fields.enable_pmi2)
      {
	printf("WARNING: Core %u fixed ctrl: %"PRIx64"\n",i,ctrl_reg.value);
	return 1;
      }
    
    if(ctrl_reg.fields.os0 != ctrl_reg.fields.usr0 ||
       ctrl_reg.fields.os1 != ctrl_reg.fields.usr1 ||
       ctrl_reg.fields.os2 != ctrl_reg.fields.usr2)
      {
	printf("WARNING: Core %u fixed ctrl: %"PRIx64"\n",i,ctrl_reg.value);
	return 1;
      }
  }
  return 0; 
}

void toggleJKTCboCounters(int currentCoreFD, int num_cbos, counterSessionInfo *session, int state) {
 int i;
  for (i = 0; i < num_cbos; i++) {
    JKTCBoxCTLRegister box_ctl;

    // start or stop all the counters in this Cbo until programming is complete
    // NOTE: these Cbo control counters are 32-bit!!!
    read_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_CTL + i*0x20, &box_ctl.value);
    box_ctl.fields.rst_ctrl = 0;
    box_ctl.fields.rst_ctrs = 0;;
    box_ctl.fields.frz = ~state;
    box_ctl.fields.frz_en = 1;
    write_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_CTL + i*0x20,&box_ctl.value);

  }
}
void toggleJKTUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state) {
  toggleJKTCboCounters(currentCoreFD,machine->num_cbos, session, state);
  //  toggleJKTimcCounters(currentCoreFD,machine->num_imc_channels, session, state);
}

void toggleNHMEXUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state) {
  NHMEXUncorePMONGlobalCtl uncoreGlobalControl;
  read_msr(currentCoreFD,U_MSR_PMON_GLOBAL_CTL, &uncoreGlobalControl.value);
  uncoreGlobalControl.fields.en_all = 1;
  uncoreGlobalControl.fields.rst_all = 0;
  uncoreGlobalControl.fields.frz_all = ~state;
  write_msr(currentCoreFD,U_MSR_PMON_GLOBAL_CTL, &uncoreGlobalControl.value);
}

void toggleIVBUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state) {
  IVBUncorePMONGlobalCtl uncoreGlobalControl;
  printf("Toggle Counters: State: %d\n",state);

  read_msr(currentCoreFD,IVB_UNC_PERF_GLOBAL_CTRL, &uncoreGlobalControl.value);
  uncoreGlobalControl.fields.en = state;
  write_msr(currentCoreFD,IVB_UNC_PERF_GLOBAL_CTRL, &uncoreGlobalControl.value);
}

// @@ Add Beckton uncore toggle here!!!
void toggleUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session, int state) {
  switch(machine->cpu_model) {
  case JAKETOWN:
    toggleJKTUncoreCounters(currentCoreFD, machine, session, state);
    break;
  case NEHALEM_EX:
    toggleNHMEXUncoreCounters(currentCoreFD, machine, session, state);
    break;
  case IVY_BRIDGE:
    toggleIVBUncoreCounters(currentCoreFD, machine, session, state);
    break;
  default:
    printf("Uncore counters for this CPU model are not yet supported\n");
  }
}

void stopCounters(int core, int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  uint64 value = 0;
  write_msr( currentCoreFD, IA32_CR_PERF_GLOBAL_CTRL, &value);
  //  if (core % machine->num_phys_cores_per_socket == 0 && core / machine->num_phys_cores_per_socket < machine->num_sockets)
  //    toggleUncoreCounters( currentCoreFD, machine, session, OFF);
    if (machine->cpu_model == JAKETOWN && core % machine->num_phys_cores_per_socket == 0 && core / machine->num_phys_cores_per_socket < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, OFF);
    else if (machine->cpu_model == NEHALEM_EX && core < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, OFF);
    else if (machine->cpu_model == IVY_BRIDGE && core < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, OFF);

}

void startCounters(int core, int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  uint64 value = 0;
  int j;
  for (j = 0; j < session->core_gen_counter_num_used; j++)
    value += (1ULL << j);
  // assuming fixed counters are always used..
  value += (1ULL << 32) + (1ULL << 33) + (1ULL << 34);
  
  write_msr( currentCoreFD, IA32_CR_PERF_GLOBAL_CTRL, &value);

    if (machine->cpu_model == JAKETOWN && core % machine->num_phys_cores_per_socket == 0 && core / machine->num_phys_cores_per_socket < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, ON);
    else if (machine->cpu_model == NEHALEM_EX && core < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, ON);
    else if (machine->cpu_model == IVY_BRIDGE && core < machine->num_sockets)
      toggleUncoreCounters( currentCoreFD, machine, session, ON);
}

  /* CORE FIXED COUNTER PROGRAMMING */
void programCoreFixedCounters(int currentCoreFD) {
  FixedEventControlRegister ctrl_reg;
  read_msr( currentCoreFD, IA32_CR_FIXED_CTR_CTRL, &ctrl_reg.value );
  
  ctrl_reg.fields.os0 = 1;
  ctrl_reg.fields.usr0 = 1;
  ctrl_reg.fields.any_thread0 = 0;
  ctrl_reg.fields.enable_pmi0 = 0;
  
  ctrl_reg.fields.os1 = 1;
  ctrl_reg.fields.usr1 = 1;
  ctrl_reg.fields.any_thread1 = 0;
  ctrl_reg.fields.enable_pmi1 = 0;
  
  ctrl_reg.fields.os2 = 1;
  ctrl_reg.fields.usr2 = 1;
  ctrl_reg.fields.any_thread2 = 0;
  ctrl_reg.fields.enable_pmi2 = 0;	
  
  write_msr( currentCoreFD, IA32_CR_FIXED_CTR_CTRL, &ctrl_reg.value );
  /* END FIXED COUNTER PROGRAMMING */
}

// Warning: Event 0xD2 on Jaketown requires a workaround in the Intel PCM code!
void programGeneralPurposeRegisters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  EventSelectRegister event_select_reg;
  uint64 value;
  int j;
  for (j = 0; j < session->core_gen_counter_num_used; j++) {
    read_msr(currentCoreFD,IA32_PERFEVTSEL0_ADDR + j, &event_select_reg.value);
    event_select_reg.fields.event_select = session->core_event_numbers[j];
    event_select_reg.fields.umask = session->core_umasks[j];
    event_select_reg.fields.usr = 1;
    event_select_reg.fields.os = 1;
    event_select_reg.fields.edge = 0;
    event_select_reg.fields.pin_control = 0;
    event_select_reg.fields.apic_int = 0;
    event_select_reg.fields.any_thread = 0;
    event_select_reg.fields.enable = 1;
    event_select_reg.fields.invert = 0;
    event_select_reg.fields.cmask = 0;
    event_select_reg.fields.in_tx = 0;
    event_select_reg.fields.in_txcp = 0;
    
    // actually program the counters
    value = 0;
    write_msr(currentCoreFD,IA32_PMC0 + j, &value);
    write_msr(currentCoreFD, IA32_PERFEVTSEL0_ADDR + j, &event_select_reg.value);
  }
}

void readJKTCboCounters(int socket, int currentCoreFD, int num_cbos, int cbo_counters_used, counterData *data) {
  int i,j;
  JKTCBoxPMONCTR tmp;
  for (i = 0; i < num_cbos; i++) {
    for (j = 0; j < cbo_counters_used; j++) {
      // these are 64-bit registers, of which 44bits is counter...
      read_msr(currentCoreFD,JKT_C0_MSR_PMON_CTR0 + j + i*0x20, &tmp.value);
      data->cboUncore[socket*num_cbos + i][j] = tmp.fields.event_count;
    }
  }
}

void readNHMEXCboCounters(int socket, int currentCoreFD, int num_cbos, int cbo_counters_used, counterData *data) {
  int i,j;
  NHMEXCboPMONCounter tmp;
  for (i = 0; i < num_cbos; i++) {
    for (j = 0; j < cbo_counters_used; j++) {
      // these are 64-bit registers, of which 48bits is counter...
      read_msr(currentCoreFD,CB0_CR_C_MSR_PMON_CTR_0 + j*2 + i*0x20, &tmp.value);
      // WARNING: order of CBos in counts is now 0,4,2,6,1,5,3,7
      data->cboUncore[socket*num_cbos + i][j] = tmp.fields.event_count;
    }
  }
}

void readIVBCboCounters(int socket, int currentCoreFD, int num_cbos, int cbo_counters_used, counterData *data) {
  int i,j;
  IVBCboCounter tmp;
  for (i = 0; i < num_cbos; i++) {
    for (j = 0; j < cbo_counters_used; j++) {
      printf("Reading cbo %d, counter %d\n",i,j);
      // these are 64-bit registers, of which 44bits is counter...
      read_msr(currentCoreFD,IVB_UNC_CBO_0_PER_CTR0 + j + i*0x10, &tmp.value);
      data->cboUncore[socket*num_cbos + i][j] = tmp.fields.event_count;
    }
  }
}

void readJKTUncoreCounters(int socket, int currentCoreFD, machineInformation *machine, counterSessionInfo *session, counterData *data) {
  readJKTCboCounters(socket, currentCoreFD, machine->num_cbos, session->cbo_counter_num_used, data);
}
void readNHMEXUncoreCounters(int socket, int currentCoreFD, machineInformation *machine, counterSessionInfo *session, counterData *data) {
  readNHMEXCboCounters(socket, currentCoreFD, machine->num_cbos, session->cbo_counter_num_used, data);
}
void readIVBUncoreCounters(int socket, int currentCoreFD, machineInformation *machine, counterSessionInfo *session, counterData *data) {
  readIVBCboCounters(socket, currentCoreFD, machine->num_cbos, session->cbo_counter_num_used, data);
}

// @@ Add Beckton uncore read here!!!!
void readUncoreCounters(int core, int currentCoreFD,machineInformation *machine, counterSessionInfo *session, counterData *data) {
    switch(machine->cpu_model) {
    case JAKETOWN:
      if (core % machine->num_phys_cores_per_socket == 0 && core / machine->num_phys_cores_per_socket < machine->num_sockets) {
	readJKTUncoreCounters(core / machine->num_phys_cores_per_socket, currentCoreFD, machine, session, data);
      }
      break;
    case NEHALEM_EX:
      if (core < machine->num_sockets)
	readNHMEXUncoreCounters(core % machine->num_sockets, currentCoreFD, machine, session, data);
      break;
    case IVY_BRIDGE:
      if (core < machine->num_sockets)
	readIVBUncoreCounters(core % machine->num_sockets, currentCoreFD, machine, session, data);
      break;
    default:
      printf("Uncore counters for this CPU model are not yet supported\n");
    }
}

void readCounters(int *fd, machineInformation *machine, counterSessionInfo *session, counterData *data) {
  int i, j;
  for (i = 0; i < machine->num_cores; i++) {
    int currentCoreFD = fd[i];
    read_msr( currentCoreFD, INST_RETIRED_ANY_ADDR, &(data->fixedCore[i].inst_retired_any_addr));
    read_msr( currentCoreFD, CPU_CLK_UNHALTED_THREAD_ADDR, &data->fixedCore[i].cpu_clk_unhalted_thread_addr);
    read_msr( currentCoreFD, CPU_CLK_UNHALTED_REF_ADDR, &data->fixedCore[i].cpu_clk_unhalted_ref_addr );
    for (j = 0; j < session->core_gen_counter_num_used; j++)
      read_msr( currentCoreFD, IA32_PMC0 + j, &(data->generalCore[i][j]));

    readUncoreCounters(i, currentCoreFD, machine, session, data);
  }
}

void diffFixedCounterData(machineInformation * machine, counterData *final, counterData *before, counterData * after) {
  int i;
  for (i = 0; i < machine->num_cores; i++) {
    final->fixedCore[i].inst_retired_any_addr = after->fixedCore[i].inst_retired_any_addr - before->fixedCore[i].inst_retired_any_addr;
    final->fixedCore[i].cpu_clk_unhalted_thread_addr = after->fixedCore[i].cpu_clk_unhalted_thread_addr - before->fixedCore[i].cpu_clk_unhalted_thread_addr;
    final->fixedCore[i].cpu_clk_unhalted_ref_addr = after->fixedCore[i].cpu_clk_unhalted_ref_addr - before->fixedCore[i].cpu_clk_unhalted_ref_addr;
  }
}

void diffGeneralCounterData(machineInformation *machine, counterSessionInfo *session, counterData *final, counterData *before, counterData *after) {
  int i,j;
  for (i = 0; i < machine->num_cores; i++) 
    for (j = 0; j < session->core_gen_counter_num_used; j++)
      final->generalCore[i][j] = (after->generalCore[i][j]-before->generalCore[i][j]);
}


void programNHMEXCbo(int currentCoreFD, int num_cbos, counterSessionInfo *session) {
  int i,j;
  for (i = 0; i < num_cbos; i++) {
    NHMEXCboPMONGlobalCtl box_ctl;
    NHMEXCboPMONEventSel ctr_ctl;
    
    read_msr(currentCoreFD,CB0_CR_C_MSR_PMON_GLOBAL_CTL + i*0x20, &box_ctl.value);
    // enable all counters on this cbo...still need to set the counter-specific enable bits
    box_ctl.fields.ctr_en = 0x2f;
    write_msr(currentCoreFD,CB0_CR_C_MSR_PMON_GLOBAL_CTL + i*0x20, &box_ctl.value);
   
    for (j = 0; j < session->cbo_counter_num_used; j++) {
      read_msr(currentCoreFD,CB0_CR_C_MSR_PMON_EVT_SEL_0 + j*2 + i*0x20, &ctr_ctl.value);
      ctr_ctl.fields.ev_sel = session->cbo_event_numbers[j];
      ctr_ctl.fields.umask = session->cbo_umasks[j];
      ctr_ctl.fields.edge_detect = 0x0;
      ctr_ctl.fields.invert = 0x0;
      ctr_ctl.fields.threshold = 0x0;
      ctr_ctl.fields.en = 0x1;
      write_msr(currentCoreFD,CB0_CR_C_MSR_PMON_EVT_SEL_0 + j*2 + i*0x20, &ctr_ctl.value);
    }
  }
}

void programIVBCbo(int currentCoreFD, int num_cbos, counterSessionInfo *session) {
  int i,j;
  IVBCboConfig cboConfig;
  read_msr(currentCoreFD,IVB_UNC_CBO_CONFIG, &cboConfig.value);
  printf("Num Cbos: %d\n",cboConfig.fields.num_cbo);
  for (i = 0; i < num_cbos; i++) {
    IVBCboCounter ctr;
    IVBCboPerfEvtSel ctr_ctl;
    ctr.value = 0x0;
    for (j = 0; j < session->cbo_counter_num_used; j++) {
      printf("Programming cbo %d, counter %d\n",i,j);
      read_msr(currentCoreFD,IVB_UNC_CBO_0_PERFEVTSEL0 + j + i*0x10, &ctr_ctl.value);
      ctr_ctl.fields.event_select = session->cbo_event_numbers[j];
      ctr_ctl.fields.umask = session->cbo_umasks[j];
      ctr_ctl.fields.edge_det = 0x0;
      ctr_ctl.fields.ov_en = 0x0;
      //    ctr_ctl.fields.cmask = 0x0;
      ctr_ctl.fields.en = 0x1;
      write_msr(currentCoreFD,IVB_UNC_CBO_0_PERFEVTSEL0 + j + i*0x10, &ctr_ctl.value);

      // reset the counter
      //  write_msr(currentCoreFD,IVB_UNC_CBO_0_PER_CTR0 + j + i*0x10, &ctr.value);
      
    }
  }
}

void programJKTCbo(int currentCoreFD, int num_cbos, counterSessionInfo *session) {
  int i,j;
  for (i = 0; i < num_cbos; i++) {
    JKTCBoxCTLRegister box_ctl;
    JKTCBoxPMONCTLRegister ctr_ctl;
    JKTCBoxPMONFilterRegister box_filter;
    // freeze all the counters in this Cbo until programming is complete
    read_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_CTL + i*0x20, &box_ctl.value);
    box_ctl.fields.rst_ctrl = 0;
    // reset counters before run...
    box_ctl.fields.rst_ctrs = 1;
    box_ctl.fields.frz = 1;
    box_ctl.fields.frz_en = 1;
    write_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_CTL + i*0x20,&box_ctl.value);

    // set the filter register for this Cbo
    read_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_FILTER + i*0x20, &box_filter.value);
    box_filter.fields.tid = 0x0;
    box_filter.fields.nid = 0xff;
    // capture all requests FMESI
    box_filter.fields.state = session->cbo_filter;
    write_msr(currentCoreFD,JKT_C0_MSR_PMON_BOX_FILTER + i*0x20, &box_filter.value);
    
    
    for (j = 0; j < session->cbo_counter_num_used; j++) {
      read_msr(currentCoreFD,JKT_C0_MSR_PMON_CTL0 + j + i*0x20, &ctr_ctl.value);
      ctr_ctl.fields.ev_sel = session->cbo_event_numbers[j];
      ctr_ctl.fields.umask = session->cbo_umasks[j];
      ctr_ctl.fields.rst = 0x0;
      ctr_ctl.fields.edge_det = 0x0;
      ctr_ctl.fields.tid_en = 0x0;
      ctr_ctl.fields.en = 0x1;
      ctr_ctl.fields.invert = 0x0;
      ctr_ctl.fields.thresh = 0x0;
      write_msr(currentCoreFD,JKT_C0_MSR_PMON_CTL0 + j + i*0x20, &ctr_ctl.value);      
    }
  }
}

void programJKTUncore(int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  programJKTCbo(currentCoreFD, machine->num_cbos, session);
 
}
void programNHMEXUncore(int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  NHMEXUncorePMONGlobalCtl uncoreGlobalControl;
  // reset the uncore counters...
  read_msr(currentCoreFD,U_MSR_PMON_GLOBAL_CTL, &uncoreGlobalControl.value);
  uncoreGlobalControl.fields.rst_all = 1;
  uncoreGlobalControl.fields.frz_all = 1;
  write_msr(currentCoreFD,U_MSR_PMON_GLOBAL_CTL, &uncoreGlobalControl.value);


  programNHMEXCbo(currentCoreFD, machine->num_cbos, session);
 
}

void programIVBUncore(int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  IVBUncorePMONGlobalCtl uncoreGlobalControl;
  // reset the uncore counters...
  read_msr(currentCoreFD,IVB_UNC_PERF_GLOBAL_CTRL, &uncoreGlobalControl.value);
  
  uncoreGlobalControl.fields.wake_pmi = 0x0;
  uncoreGlobalControl.fields.en = 0x0;
  uncoreGlobalControl.fields.frz = 0x0;
  write_msr(currentCoreFD,IVB_UNC_PERF_GLOBAL_CTRL, &uncoreGlobalControl.value);


  programIVBCbo(currentCoreFD, machine->num_cbos, session);
 
}

void programUncoreCounters(int currentCoreFD, machineInformation *machine, counterSessionInfo *session) {
  switch(machine->cpu_model) {
  case JAKETOWN:
    programJKTUncore(currentCoreFD, machine, session);
    break;
  case NEHALEM_EX:
    programNHMEXUncore(currentCoreFD,machine,session);
    break;
  case IVY_BRIDGE:
    programIVBUncore(currentCoreFD,machine,session);
    break;
  default:
    printf("Uncore counters for this CPU model are not yet supported\n");
  }
}

void cleanup(int *fd, machineInformation *machine, counterSessionInfo *session) {
  int i,j;
 for (i = 0; i < machine->num_cores; i++) {
    int currentCoreFD = fd[i];
    uint64 value;
    
    value = (1ULL << 32) + (1ULL << 33) + (1ULL << 34);
    write_msr( currentCoreFD, IA32_CR_PERF_GLOBAL_CTRL, &value );
    
    // reset general purpose counters
    for (j = 0; j < machine->core_gen_counter_num_max; j++) {
      value = 0;
      write_msr( currentCoreFD, IA32_PERFEVTSEL0_ADDR + j, &value);
    }
    
    if (machine->cpu_model == JAKETOWN &&i % machine->num_phys_cores_per_socket == 0 && i / machine->num_phys_cores_per_socket < machine->num_sockets) 
      toggleUncoreCounters(currentCoreFD,machine, session, OFF);
    else if (machine->cpu_model == NEHALEM_EX && i < machine->num_sockets)
      toggleUncoreCounters(currentCoreFD,machine, session, OFF);
    else if (machine->cpu_model == IVY_BRIDGE && i < machine->num_sockets)
      toggleUncoreCounters(currentCoreFD,machine, session, OFF);
    close_msr_file( &currentCoreFD );

 }
}

void diffJKTCboCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  int i, j;
  int32 num_cbos = machine->num_cbos;

  for (i = 0; i < machine->num_sockets; i++)
    for (j = 0; j < num_cbos; j++) {
      // should really be a third loop over the number of used cbo counters...
      final->cboUncore[i*num_cbos + j][0] = after->cboUncore[i*num_cbos + j][0] - before->cboUncore[i*num_cbos + j][0];
      final->cboUncore[i*num_cbos + j][1] = after->cboUncore[i*num_cbos + j][1] - before->cboUncore[i*num_cbos + j][1];
    }
}

// could be merged with the above function...
void diffIVBCboCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  int i, j;
  int32 num_cbos = machine->num_cbos;

  for (i = 0; i < machine->num_sockets; i++)
    for (j = 0; j < num_cbos; j++) {
      // should really be a third loop over the number of used cbo counters...
      final->cboUncore[i*num_cbos + j][0] = after->cboUncore[i*num_cbos + j][0] - before->cboUncore[i*num_cbos + j][0];
      final->cboUncore[i*num_cbos + j][1] = after->cboUncore[i*num_cbos + j][1] - before->cboUncore[i*num_cbos + j][1];
    }
}
// could be merged with the above function...
void diffNHMEXCboCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  int i, j;
  int32 num_cbos = machine->num_cbos;

  for (i = 0; i < machine->num_sockets; i++)
    for (j = 0; j < num_cbos; j++) {
      // should really be a third loop over the number of used cbo counters...
      final->cboUncore[i*num_cbos + j][0] = after->cboUncore[i*num_cbos + j][0] - before->cboUncore[i*num_cbos + j][0];
      final->cboUncore[i*num_cbos + j][1] = after->cboUncore[i*num_cbos + j][1] - before->cboUncore[i*num_cbos + j][1];
    }
}

void diffJKTUncoreCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  diffJKTCboCounters(machine,session,final,before,after);
}

void diffNHMEXUncoreCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  diffNHMEXCboCounters(machine,session,final,before,after);
}

void diffIVBUncoreCounters(machineInformation *machine, counterSessionInfo *session, counterData *final,counterData *before,counterData *after) {
  diffIVBCboCounters(machine,session,final,before,after);
}

// @@ ADD Beckton uncore diff here!!!
void diffUncoreCounterData(machineInformation *machine, counterSessionInfo *session, counterData* final, counterData *before, counterData *after) {
  switch(machine->cpu_model) {
  case JAKETOWN:
    diffJKTUncoreCounters(machine,session,final,before,after);
    break;
  case NEHALEM_EX:
    diffNHMEXUncoreCounters(machine,session,final,before,after);
    break;
  case IVY_BRIDGE:
    diffIVBUncoreCounters(machine,session,final,before,after);
    break;
    
  default:
    printf("Uncore counters for this CPU model are not yet supported\n");
  }
}
 
void diffCounterData(machineInformation *machine, counterSessionInfo *session, counterData* final, counterData *before, counterData *after) {
  diffFixedCounterData(machine,final,before,after);
  diffGeneralCounterData(machine,session, final,before,after);
  diffUncoreCounterData(machine, session, final, before, after);
}

