#include <AllwinnerTimer.hpp>

struct AllwinnerHSTimer {
    static const struct Instance {
        volatile AllwinnerHSTimer* address;
        Natural8 interruptPort;
    } instances[];

    union {
        struct {
            Natural32 hsTimer : 1;
        };
        Natural32 raw;
    } timerIRQEnable;
    union {
        struct {
            Natural32 hsTimer : 1;
        };
        Natural32 raw;
    } timerIRQStatus;
    Natural32 pad0[2];
    union {
        struct {
            Natural32 enable : 1,
                      reload : 1,
                      pad0 : 2,
                      preScale : 3,
                      disableOnReachingZero : 1,
                      pad1 : 23,
                      testMode : 1;
        };
        Natural32 raw;
    } control;
    Natural64 intervalValue, currentValue;
};
