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
    Natural32 pad0,
              debugControl[2];
    struct {
        Natural32 bandwidthLimit,
                  port;
    } masterConfig[12];
    Natural32 pad1[8],
              bandwidthControl,
              unknown0[475],
              MCProtect;

    Natural8 getDRAMSize() volatile {
        return control.rank+control.bank+control.rowWidth+control.pageSize+6;
    }
};
