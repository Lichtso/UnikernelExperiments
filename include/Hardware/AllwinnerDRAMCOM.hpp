#include <Hardware/AXP803.hpp>

struct AllwinnerDRAMCOM {
    static const struct Instance {
        volatile AllwinnerDRAMCOM* address;
    } instances[];

    union {
        struct {
            Natural32 rank : 2,
                      bank : 2,
                      rowWidth : 4,
                      pageSize : 4,
                      DQWidth : 3,
                      unknown0 : 1,
                      DRAMType : 3,
                      unknown1 : 1,
                      unknown2 : 8;
        };
        Natural32 raw;
    } control;
    Natural32 unknown0[3];
    struct {
        Natural32 bandwidthLimit,
                  port;
    } masterConfig[12];
    Natural32 unknown1[485];
};
