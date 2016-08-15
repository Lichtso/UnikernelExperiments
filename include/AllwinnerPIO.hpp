#include <AllwinnerPRCM.hpp>

struct AllwinnerPIO {
    static const struct Instance {
        volatile AllwinnerPIO* address;
    } instances[];

    struct {
        union {
            struct {
                Natural32 slot0 : 3, pad0 : 1, slot1 : 3, pad1 : 1,
                          slot2 : 3, pad2 : 1, slot3 : 3, pad3 : 1,
                          slot4 : 3, pad4 : 1, slot5 : 3, pad5 : 1,
                          slot6 : 3, pad6 : 1, slot7 : 3, pad7 : 1;
            };
            Natural32 raw;
        } configure[4];

        Natural32 data;
        union {
            struct {
                Natural32 slot0 : 2, slot1 : 2, slot2 : 2, slot3 : 2,
                          slot4 : 2, slot5 : 2, slot6 : 2, slot7 : 2,
                          slot8 : 2, slot9 : 2, slot10 : 2, slot11 : 2,
                          slot12 : 2, slot13 : 2, slot14 : 2, slot15 : 2;
            };
            Natural32 raw;
        } multiDriving[2];
        union {
            struct {
                Natural32 slot0 : 2, slot1 : 2, slot2 : 2, slot3 : 2,
                          slot4 : 2, slot5 : 2, slot6 : 2, slot7 : 2,
                          slot8 : 2, slot9 : 2, slot10 : 2, slot11 : 2,
                          slot12 : 2, slot13 : 2, slot14 : 2, slot15 : 2;
            };
            Natural32 raw;
        } pull[2];
    } banks[8];
    Natural8 pad0[224];
    struct {
        Natural32
            PIOInterrruptConfigure[4],
            PIOInterruptControl,
            PIOInterruptStatus,
            PIOInterruptDebounce,
            pad0;
    } interruptBanks[3];
};
