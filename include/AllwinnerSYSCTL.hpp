#include <AllwinnerCPUCFG.hpp>

struct AllwinnerSYSCTL {
    static const struct Instance {
        volatile AllwinnerSYSCTL* address;
    } instances[];

    Natural32 pad0[9];
    union {
        struct {
            Natural32 version : 8,
                      bootSelectPinStatus : 1;
        };
        Natural32 raw;
    } versionRegister;
    Natural32 pad1[2];
    union {
        struct {
            Natural32 clockSource : 2,
                      phyInterface : 1,
                      invertTransmitClock : 1,
                      invertReceiveClock : 1,
                      receiveClockDelayChain : 5,
                      transmitClockDelayChain : 3,
                      RMIIEnable : 1;
        };
        Natural32 raw;
    } EMACClock;
};
