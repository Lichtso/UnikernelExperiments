#include "AllwinnerTimer.hpp"

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
    } timerIRQEnable,
      timerIRQStatus;
    Natural32 pad0[2];
    union {
        struct {
            Natural32 enable : 1,
                      reload : 1,
                      pad0 : 2,
                      preScale : 3, // Base Frequency: 200 MHz
                      oneShot : 1,
                      pad1 : 23,
                      testMode : 1;
        };
        Natural32 raw;
    } control;
    Natural32 intervalValueLow, intervalValueHigh,
              currentValueLow, currentValueHigh;

    void initialize() volatile {
        load(0, false);
    }

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

    void printUptime() volatile {
        auto UART = AllwinnerUART::instances[0].address;
        UART->puts("Uptime: ");
        UART->putHex(-(getCurrentValue()|(0xFFULL<<56)));
        UART->putc('\n');
    }
};
