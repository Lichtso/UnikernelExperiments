#include "GICPL400.hpp"

struct AllwinnerTimer {
    static const struct Instance {
        volatile AllwinnerTimer* address;
        Natural8 interruptPorts[2];
    } instances[];

    union {
        struct {
            Natural32 timer0 : 1,
                      timer1 : 1;
        };
        Natural32 raw;
    } timerIRQEnable;
    union {
        struct {
            Natural32 timer0 : 1,
                      timer1 : 1;
        };
        Natural32 raw;
    } timerIRQStatus;
    Natural32 pad0[2];
    struct {
        union {
            struct {
                Natural32 enable : 1,
                          reload : 1,
                          clockSource : 2,
                          preScale : 3,
                          mode : 1;
            };
            Natural32 raw;
        } control;
        Natural32 intervalValue, currentValue, pad0;
    } timer[2];
    Natural32 pad1[20];
    union {
        struct {
            Natural32 counter0Enable : 1,
                      counter1Enable : 1,
                      pad0 : 6,
                      counter0Pause : 1,
                      counter1Pause : 1;
        };
        Natural32 raw;
    } AVSControl;
    Natural32 AVSCounter[2];
    union {
        struct {
            Natural32 counter0 : 12,
                      pad0 : 4,
                      counter1 : 12;
        };
        Natural32 raw;
    } AVSDivisor;
    Natural32 pad2[4];
    union {
        struct {
            Natural32 enable : 1;
        };
        Natural32 raw;
    } watchdogIRQEnable;
    union {
        struct {
            Natural32 pending : 1;
        };
        Natural32 raw;
    } watchdogIRQStatus;
    Natural32 pad3[2];
    union {
        struct {
            Natural32 restart : 1,
                      key : 12; // 0xA57
        };
        Natural32 raw;
    } watchdogControl;
    union {
        struct {
            Natural32 configuration : 2;
        };
        Natural32 raw;
    } watchdogConfiguration;
    union {
        struct {
            Natural32 enable : 1,
                      pad : 3,
                      intervalValue : 4;
        };
        Natural32 raw;
    } watchdogMode;
};
