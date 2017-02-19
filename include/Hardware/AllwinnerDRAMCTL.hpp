#include <Hardware/AllwinnerDRAMCOM.hpp>

struct AllwinnerDRAMCTL {
    static const struct Instance {
        volatile AllwinnerDRAMCTL* address;
    } instances[];

    Natural32
        PI,
        powerControl,
        modeControl,
        clockEnable,
        PGS[2],
        status,
        pad0[5],
        mode[4],
        PLLGC,
        PT[5],
        timing[9],
        odtConfig,
        piTiming[2],
        pad1,
        refreshControl0,
        refreshTiming,
        refreshControl1,
        powerTiming,
        pad2[7],
        unknown0,
        DQSGM,
        DTC,
        DTA[4],
        DTD[2],
        DTM[2],
        DTBM,
        CAT[2],
        DTED[2],
        pad3[2],
        PGC[4],
        IOVC[2],
        DQSD,
        DXCC,
        odtMap,
        ZQCTL[2],
        pad4[5];
    struct {
        Natural32 C, P, D, S;
    } ZQ[8];
    Natural32
        SCHED,
        peripheralFHP[2],
        peripheralFLP[2],
        peripheralW[2],
        pad5[9],
        ACMDL,
        ACLCDL,
        ACIOC,
        pad6,
        CAIOC[60];
    struct {
        Natural32
            MDL,
            LCDL[3],
            DATXIOC[11],
            BDL6,
            GT,
            GC,
            GS[3],
            pad0[11];
    } DX[4];
    Natural32
        pad7[192],
        BISTR,
        BISTWC,
        pad8[3],
        BISTLS,
        pad9[4],
        BISTGS,
        BISTWE,
        BISTBE[4],
        BISTWCS,
        BISTFW[3],
        pad10[12],
        MXUpdate[3];
};
