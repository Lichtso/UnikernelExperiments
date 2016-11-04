#include <Hardware/AllwinnerCCU.hpp>

struct GICPL400 {
    static const struct Instance {
        volatile GICPL400* address;
    } instances[];

    Natural32 pad0[1024];
    union {
        struct {
            Natural32 enable : 1;
        };
        Natural32 raw;
    } distributorControl;
    union {
        struct {
            Natural32 ITLinesNumber : 5,
                      CPUNumber : 3,
                      pad0 : 2,
                      securityExtensions : 1,
                      LSPI : 5;
        };
        Natural32 raw;
    } interruptControllerType;
    Natural32 distributorImplementerIdentification,
              pad1[29],
              interruptSecurity[32],
              interruptSetEnable[32],
              interruptClearEnable[32],
              interruptSetPending[32],
              interruptClearPending[32],
              activeBit[32],
              pad2[32],
              interruptPriority[256],
              interruptProcessorTargets[256],
              interruptConfiguration[64],
              privatePeripheralInterruptStatus,
              sharedPeripheralInterruptStatus[15],
              pad3[112];
    union {
        struct {
            Natural32 SGIInterruptId : 4,
                      pad0 : 11,
                      SATT : 1,
                      CPUTargetList : 8,
                      targetListFilter : 2;
        };
        Natural32 raw;
    } softwareGeneratedInterrupt;
    Natural32 softwareGeneratedInterruptClearPending,
              softwareGeneratedInterruptSetPending,
              pad4[61],
              CPUInterfaceControl,
              interruptPriorityMask,
              binaryPoint,
              interruptAcknowledge,
              endOfInterrupt,
              runningPriority,
              highestPendingInterrupt,
              aliasedBinaryPoint,
              aliasedInterruptAcknowledge,
              aliasedEndOfInterrupt,
              aliasedHighestPriorityPendingInterrupt,
              pad5[41],
              activePriority,
              pad6[3],
              nonSecureActivePriority,
              pad7[6],
              CPUInterfaceIdentification,
              pad8[960],
              deactivateInterrupt;
};
