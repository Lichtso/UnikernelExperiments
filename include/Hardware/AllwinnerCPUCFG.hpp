#include "AllwinnerPIO.hpp"

struct AllwinnerCPUCFG {
    static const struct Instance {
        volatile AllwinnerCPUCFG* address;
    } instances[];

    union {
        struct {
            Natural32 pad0 : 4,
                      L2ResetDisable : 1,
                      pad1 : 3,
                      CP15SDisable : 4,
                      pad2 : 12,
                      AArch64AfterColdReset : 4,
                      cacheMaintenanceBroadcastingDownstream : 1,
                      broadcastOuterShareableTransactions : 1,
                      broadcastInnerShareableTransactions : 1,
                      broadcastBarriersSystemBusDisable : 1;
        };
        Natural32 raw;
    } clusterControl0;
    union {
        struct {
            Natural32 snoopInterfaceInactive : 1;
        };
        Natural32 raw;
    } clusterControl1;
    union {
        struct {
            Natural32 L1IDDelay : 3, pad0 : 1,
                      L1ITDelay : 3, pad1 : 1,
                      L1DDDelay : 3, pad2 : 1,
                      L1DTDelay : 3, pad3 : 1,
                      L1DYDelay : 3, pad4 : 1,
                      BTACDelay : 3, pad5 : 1,
                      L1TLBDelay : 3, pad6 : 1,
                      L1SDTDelay : 3;
        };
        Natural32 raw;
    } cacheConfig0;
    union {
        struct {
            Natural32 pad0 : 4,
                      L2TDelay : 3, pad1 : 5,
                      L2VDelay : 3, pad2 : 1,
                      EMA : 3, pad3 : 5,
                      EMAW : 2;
        };
        Natural32 raw;
    } cacheConfig1;
    Natural32 pad0[6];
    union {
        struct {
            Natural32 pad0 : 4,
                      GICCDisable : 1,
                      pad1 : 3,
                      L2FlushRequest : 1,
                      pad2 : 3,
                      cryptoDisable : 4,
                      externalMonitorRequestClear : 1,
                      pad3 : 3,
                      EXMClear : 4,
                      eventInput : 1;
        };
        Natural32 raw;
    } generalControl;
    Natural32 pad1;
    union {
        struct {
            Natural32 StandByWFIL2 : 1,
                      pad0 : 7,
                      StandByWFE : 4,
                      pad1 : 4,
                      StandByWFI : 4,
                      pad2 : 4,
                      SMP : 4;
        };
        Natural32 raw;
    } clusterCPUStatus;
    Natural32 pad2[2];
    union {
        struct {
            Natural32 pad0 : 8,
                      externalMonitorRequestClearAcknowledge : 1,
                      eventOutput : 1,
                      L2FlushDone : 1;
        };
        Natural32 raw;
    } L2Status;
    Natural32 pad3[16];
    union {
        struct {
            Natural32 coreReset : 4,
                      pad0 : 4,
                      L2Reset : 1,
                      pad1 : 3,
                      HReset : 1,
                      pad2 : 7,
                      MBISTReset : 1,
                      pad3 : 3,
                      socDebugReset : 1,
                      pad4 : 3,
                      DDRReset : 1;
        };
        Natural32 raw;
    } clusterResetControl;
    Natural32 pad4[7];
    Natural64 resetVector[4];
};
