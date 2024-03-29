#ifndef COUNTER_HOME_BREW_REGISTERS
#define COUNTER_HOME_BREW_REGISTERS

// global PMU control register
#define IA32_CR_PERF_GLOBAL_CTRL        0x38F

// control register for fixed counters
#define IA32_CR_FIXED_CTR_CTRL          0x38D
// fixed register locations
#define INST_RETIRED_ANY_ADDR           (0x309)
#define CPU_CLK_UNHALTED_THREAD_ADDR    (0x30A)
#define CPU_CLK_UNHALTED_REF_ADDR       (0x30B)


// general purpose register event selectors
#define IA32_PERFEVTSEL0_ADDR           0x186
#define IA32_PERFEVTSEL1_ADDR           (IA32_PERFEVTSEL0_ADDR + 1)
#define IA32_PERFEVTSEL2_ADDR           (IA32_PERFEVTSEL0_ADDR + 2)
#define IA32_PERFEVTSEL3_ADDR           (IA32_PERFEVTSEL0_ADDR + 3)
// general purpose register locations
#define IA32_PMC0                       (0xC1)
#define IA32_PMC1                       (0xC1 + 1)
#define IA32_PMC2                       (0xC1 + 2)
#define IA32_PMC3                       (0xC1 + 3)
#define IA32_PMC4                       (0xC1 + 4)
#define IA32_PMC5                       (0xC1 + 5)
#define IA32_PMC6                       (0xC1 + 6)
#define IA32_PMC7                       (0xC1 + 7)

// Event IDs

// Nehalem/Westmere on-core events
#define MEM_LOAD_RETIRED_L3_MISS_EVTNR  (0xCB)
#define MEM_LOAD_RETIRED_L3_MISS_UMASK  (0x10)

#define MEM_LOAD_RETIRED_L3_UNSHAREDHIT_EVTNR   (0xCB)
#define MEM_LOAD_RETIRED_L3_UNSHAREDHIT_UMASK   (0x04)

#define MEM_LOAD_RETIRED_L2_HITM_EVTNR  (0xCB)
#define MEM_LOAD_RETIRED_L2_HITM_UMASK  (0x08)

#define MEM_LOAD_RETIRED_L2_HIT_EVTNR   (0xCB)
#define MEM_LOAD_RETIRED_L2_HIT_UMASK   (0x02)

/* Sandy Bridge on-core events */

// L1 <-> L2?
#define L2_RQSTS_EVTNR (0x24)
#define L2_RQSTS_DEMAND_DATA_RD_HIT_UMASK (0x01)
#define L2_RQSTS_ALL_DEMAND_DATA_RD_UMASK (0x03)
#define L2_RQSTS_ALL_RFO_UMASK (0x0C)
#define L2_RQSTS_ALL_PF_UMASK (0x24)
#define L2_L1D_WB_RQSTS_EVTNR (0x28)
#define L2_L1D_WB_RQSTS_ALL_UMASK (0x0F)

// L2 <-> L3
#define L2_LINES_IN_EVTNR (0xF1)
#define L2_LINES_IN_ALL_UMASK (0x07)
#define L2_LINES_OUT_EVTNR (0xF2)
#define L2_LINES_OUT_DEMAND_CLEAN_UMASK (0x01)
#define L2_LINES_OUT_PF_CLEAN_UMASK (0x04)
#define L2_LINES_OUT_DIRTY_ALL_UMASK (0x0A)

#define L2_TRANS_EVTNR (0xF0)
#define L2_TRANS_L2_FILL_UMASK (0x20)
#define L2_TRANS_ALL_REQUESTS_UMASK (0x80)


#define MEM_LOAD_UOPS_MISC_RETIRED_LLC_MISS_EVTNR (0xD4)
#define MEM_LOAD_UOPS_MISC_RETIRED_LLC_MISS_UMASK (0x02)

#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_NONE_EVTNR (0xD2)
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_NONE_UMASK (0x08)

#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_HITM_EVTNR (0xD2)
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_HITM_UMASK (0x04)

#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_EVTNR (0xD2)
#define MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_UMASK (0x07)

#define MEM_LOAD_UOPS_RETIRED_L2_HIT_EVTNR (0xD1)
#define MEM_LOAD_UOPS_RETIRED_L2_HIT_UMASK (0x02)

// floating point stuff
#define FP_COMP_OPS_EXE_EVTNR (0x10)
#define FP_COMP_OPS_EXE_PACKED_DOUBLE_UMASK (0x10)
#define FP_COMP_OPS_EXE_SCALAR_SINGLE_UMASK (0x20)
#define FP_COMP_OPS_EXE_PACKED_SINGLE_UMASK (0x40)
#define FP_COMP_OPS_EXE_SCALAR_DOUBLE_UMASK (0x80)

#define SIMD_FP_256_EVTNR (0x11)
#define SIMD_FP_256_PACKED_SINGLE_UMASK (0x01)
#define SIMD_FP_256_PACKED_DOUBLE_UMASK (0x02)

// architectural on-core events

#define ARCH_LLC_REFERENCE_EVTNR        (0x2E)
#define ARCH_LLC_REFERENCE_UMASK        (0x4F)

#define ARCH_LLC_MISS_EVTNR     (0x2E)
#define ARCH_LLC_MISS_UMASK     (0x41)

// Atom on-core events

#define ATOM_MEM_LOAD_RETIRED_L2_HIT_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_HIT_UMASK   (0x01)

#define ATOM_MEM_LOAD_RETIRED_L2_MISS_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_MISS_UMASK   (0x02)

#define ATOM_MEM_LOAD_RETIRED_L2_HIT_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_HIT_UMASK   (0x01)

#define ATOM_MEM_LOAD_RETIRED_L2_MISS_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_MISS_UMASK   (0x02)

#define ATOM_MEM_LOAD_RETIRED_L2_HIT_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_HIT_UMASK   (0x01)

#define ATOM_MEM_LOAD_RETIRED_L2_MISS_EVTNR   (0xCB)
#define ATOM_MEM_LOAD_RETIRED_L2_MISS_UMASK   (0x02)

// JKT Uncore counters...
#define MC_CH0_REGISTER_DEV (16)
#define MC_CH1_REGISTER_DEV (16)
#define MC_CH2_REGISTER_DEV (16)
#define MC_CH3_REGISTER_DEV (16)
#define MC_CH0_REGISTER_FUNC (0)
#define MC_CH1_REGISTER_FUNC (1)
#define MC_CH2_REGISTER_FUNC (4)
#define MC_CH3_REGISTER_FUNC (5)

#define MC_CH_PCI_PMON_BOX_CTL (0x0F4)

#define MC_CH_PCI_PMON_BOX_CTL_RST_CONTROL 	(1<<0)
#define MC_CH_PCI_PMON_BOX_CTL_RST_COUNTERS 	(1<<1)
#define MC_CH_PCI_PMON_BOX_CTL_FRZ 	(1<<8)
#define MC_CH_PCI_PMON_BOX_CTL_FRZ_EN 	(1<<16)

#define MC_CH_PCI_PMON_FIXED_CTL (0x0F0)

#define MC_CH_PCI_PMON_FIXED_CTL_RST (1<<19) 
#define MC_CH_PCI_PMON_FIXED_CTL_EN (1<<22) 

#define MC_CH_PCI_PMON_CTL3 (0x0E4)
#define MC_CH_PCI_PMON_CTL2 (0x0E0)
#define MC_CH_PCI_PMON_CTL1 (0x0DC)
#define MC_CH_PCI_PMON_CTL0 (0x0D8)

#define MC_CH_PCI_PMON_CTL_EVENT(x) (x<<0)
#define MC_CH_PCI_PMON_CTL_UMASK(x) (x<<8)
#define MC_CH_PCI_PMON_CTL_RST (1<<17)
#define MC_CH_PCI_PMON_CTL_EDGE_DET (1<<18)
#define MC_CH_PCI_PMON_CTL_EN (1<<22)
#define MC_CH_PCI_PMON_CTL_INVERT (1<<23)
#define MC_CH_PCI_PMON_CTL_THRESH(x) (x<<24UL)

#define MC_CH_PCI_PMON_FIXED_CTR (0x0D0)

#define MC_CH_PCI_PMON_CTR3 (0x0B8)
#define MC_CH_PCI_PMON_CTR2 (0x0B0)
#define MC_CH_PCI_PMON_CTR1 (0x0A8)
#define MC_CH_PCI_PMON_CTR0 (0x0A0)

#define QPI_PORT0_REGISTER_DEV  (8)
#define QPI_PORT0_REGISTER_FUNC (2)
#define QPI_PORT1_REGISTER_DEV  (9)
#define QPI_PORT1_REGISTER_FUNC (2)

#define QPI_PORT0_MISC_REGISTER_DEV  (8)
#define QPI_PORT0_MISC_REGISTER_FUNC (0)

#define Q_P_PCI_PMON_BOX_CTL (0x0F4)

#define Q_P_PCI_PMON_CTL3 (0x0E4)
#define Q_P_PCI_PMON_CTL2 (0x0E0)
#define Q_P_PCI_PMON_CTL1 (0x0DC)
#define Q_P_PCI_PMON_CTL0 (0x0D8)

#define Q_P_PCI_PMON_CTR3 (0x0B8)
#define Q_P_PCI_PMON_CTR2 (0x0B0)
#define Q_P_PCI_PMON_CTR1 (0x0A8)
#define Q_P_PCI_PMON_CTR0 (0x0A0)

#define Q_P_PCI_PMON_BOX_CTL_RST_CONTROL  	(1<<0)
#define Q_P_PCI_PMON_BOX_CTL_RST_COUNTERS 	(1<<1)
#define Q_P_PCI_PMON_BOX_CTL_RST_FRZ 	(1<<8)
#define Q_P_PCI_PMON_BOX_CTL_RST_FRZ_EN 	(1<<16)

#define Q_P_PCI_PMON_CTL_EVENT(x) 	(x<<0)
#define Q_P_PCI_PMON_CTL_UMASK(x) 	(x<<8)
#define Q_P_PCI_PMON_CTL_RST 		(1<<17)
#define Q_P_PCI_PMON_CTL_EDGE_DET 	(1<<18)
#define Q_P_PCI_PMON_CTL_EVENT_EXT 	(1<<21)
#define Q_P_PCI_PMON_CTL_EN 		(1<<22)
#define Q_P_PCI_PMON_CTL_INVERT 	(1<<23)
#define Q_P_PCI_PMON_CTL_THRESH(x) 	(x<<24UL)

#define QPI_RATE_STATUS (0x0D4)

#define PCU_MSR_PMON_CTR3 (0x0C39)
#define PCU_MSR_PMON_CTR2 (0x0C38)
#define PCU_MSR_PMON_CTR1 (0x0C37)
#define PCU_MSR_PMON_CTR0 (0x0C36)

#define PCU_MSR_PMON_BOX_FILTER (0x0C34)

#define PCU_MSR_PMON_BOX_FILTER_BAND_0(x) (x<<0)
#define PCU_MSR_PMON_BOX_FILTER_BAND_1(x) (x<<8)
#define PCU_MSR_PMON_BOX_FILTER_BAND_2(x) (x<<16)
#define PCU_MSR_PMON_BOX_FILTER_BAND_3(x) (x<<24)

#define PCU_MSR_PMON_CTL3 (0x0C33)
#define PCU_MSR_PMON_CTL2 (0x0C32)
#define PCU_MSR_PMON_CTL1 (0x0C31)
#define PCU_MSR_PMON_CTL0 (0x0C30)

#define PCU_MSR_PMON_BOX_CTL (0x0C24)

#define PCU_MSR_PMON_BOX_CTL_RST_CONTROL (1<<0)
#define PCU_MSR_PMON_BOX_CTL_RST_COUNTERS (1<<1)
#define PCU_MSR_PMON_BOX_CTL_FRZ (1<<8)
#define PCU_MSR_PMON_BOX_CTL_FRZ_EN (1<<16)

#define PCU_MSR_PMON_CTL_EVENT(x) (x<<0)
#define PCU_MSR_PMON_CTL_OCC_SEL(x) (x<<14)
#define PCU_MSR_PMON_CTL_RST	(1<<17)
#define PCU_MSR_PMON_CTL_EDGE_DET (1<<18)
#define PCU_MSR_PMON_CTL_EXTRA_SEL (1<<21)
#define PCU_MSR_PMON_CTL_EN	(1<<22)
#define PCU_MSR_PMON_CTL_INVERT (1<<23)
#define PCU_MSR_PMON_CTL_THRESH(x) (x<<24UL)
#define PCU_MSR_PMON_CTL_OCC_INVERT (1UL<<30UL)
#define PCU_MSR_PMON_CTL_OCC_EDGE_DET (1UL<<31UL)

#define JKT_C0_MSR_PMON_CTR3        0x0D19 // CBo 0 PMON Counter 3
#define JKT_C0_MSR_PMON_CTR2        0x0D18 // CBo 0 PMON Counter 2
#define JKT_C0_MSR_PMON_CTR1        0x0D17 // CBo 0 PMON Counter 1
#define JKT_C0_MSR_PMON_CTR0        0x0D16 // CBo 0 PMON Counter 0
#define JKT_C0_MSR_PMON_BOX_FILTER  0x0D14 // CBo 0 PMON Filter
#define JKT_C0_MSR_PMON_CTL3        0x0D13 // CBo 0 PMON Control for Counter 3
#define JKT_C0_MSR_PMON_CTL2        0x0D12 // CBo 0 PMON Control for Counter 2
#define JKT_C0_MSR_PMON_CTL1        0x0D11 // CBo 0 PMON Control for Counter 1
#define JKT_C0_MSR_PMON_CTL0        0x0D10 // CBo 0 PMON Control for Counter 0
#define JKT_C0_MSR_PMON_BOX_CTL     0x0D04 // CBo 0 PMON Box-Wide Control

#define CBO_MSR_PMON_BOX_CTL_RST_CONTROL (1<<0)
#define CBO_MSR_PMON_BOX_CTL_RST_COUNTERS (1<<1)
#define CBO_MSR_PMON_BOX_CTL_FRZ (1<<8)
#define CBO_MSR_PMON_BOX_CTL_FRZ_EN (1<<16)

#define CBO_MSR_PMON_CTL_EVENT(x) (x<<0)
#define CBO_MSR_PMON_CTL_UMASK(x) (x<<8)
#define CBO_MSR_PMON_CTL_RST	(1<<17)
#define CBO_MSR_PMON_CTL_EDGE_DET (1<<18)
#define CBO_MSR_PMON_CTL_TID_EN (1<<19)
#define CBO_MSR_PMON_CTL_EN	(1<<22)
#define CBO_MSR_PMON_CTL_INVERT (1<<23)
#define CBO_MSR_PMON_CTL_THRESH(x) (x<<24UL)

#define JKT_CBO_MSR_PMON_BOX_FILTER_OPC(x) (x<<23UL)

#define MSR_PACKAGE_THERMAL_STATUS (0x01B1)
#define MSR_IA32_THERMAL_STATUS    (0x019C)
#define PCM_INVALID_THERMAL_HEADROOM ((std::numeric_limits<int32>::min)())

#define MSR_DRAM_ENERGY_STATUS (0x0619)

#define MSR_PKG_C2_RESIDENCY    (0x60D)
#define MSR_PKG_C3_RESIDENCY    (0x3F8)
#define MSR_PKG_C6_RESIDENCY    (0x3F9)
#define MSR_PKG_C7_RESIDENCY    (0x3FA)
#define MSR_CORE_C3_RESIDENCY   (0x3FC)
#define MSR_CORE_C6_RESIDENCY   (0x3FD)
#define MSR_CORE_C7_RESIDENCY   (0x3FE)

#define CBO_LLC_LOOKUP (0x34)
#define CBO_LLC_LOOKUP_READS (0x3)
#define CBO_LLC_LOOKUP_WRITES (0x5)

#endif
