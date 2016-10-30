#include "AllwinnerTimer.hpp"

struct AllwinnerHSTimer {
    static const struct Instance {
        volatile AllwinnerHSTimer* address;
        Natural8 interruptPort;
    } instances[];
    static const Natural32 baseFrequency = 24000000; // AHB1 CLK

    union {
        struct {
            Natural32 hsTimer : 1;
        };
        Natural32 raw;
    } timerIRQEnable,
      timerIRQStatus;
    Natural32 pad0[2];
    union {
        struct {
            Natural32 enable : 1,
                      reload : 1,
                      pad0 : 2,
                      preScale : 3,
                      oneShot : 1,
                      pad1 : 23,
                      testMode : 1;
        };
        Natural32 raw;
    } control;
    Natural32 intervalValueLow, intervalValueHigh,
              currentValueLow, currentValueHigh;

    void load(Natural64 value, bool oneShot) volatile {
        control.raw &= ~3;
        intervalValueLow = static_cast<Natural32>(value);
        intervalValueHigh = static_cast<Natural32>(value>>32);
        control.preScale = 0;
        control.oneShot = oneShot;
        control.raw |= 3;
    }

    Natural64 getCurrentValue() volatile {
        Natural64 currentValue = currentValueLow;
        currentValue |= (static_cast<Natural64>(currentValueHigh)<<32);
        return currentValue;
    }
};
