#include <AllwinnerSYSCTL.hpp>

struct AllwinnerPRCM {
    static const struct Instance {
        volatile AllwinnerPRCM* address;
    } instances[];

    Natural32
        CPUSReset,
        Cluster0CPUXResetControl,
        Cluster1CPUXResetControl,
        pad0,
        CPUSClockConfiguration,
        pad1[2],
        APBSClockDivide,
        pad2[2],
        APBSClockGating,
        pad3[6],
        PLLControl,
        pad4[2],
        R_ONE_WIREClock,
        R_CIR_RXClock,
        R_DAUDIO0Clock,
        R_DAUDIO1Clock,
        pad5[20],
        APBSSoftwareReset,
        pad6[19],
        Cluster0CPUXPowerOffGating,
        Cluster1CPUXPowerOffGating,
        pad7[2],
        VDD_SYSPowerOffGating,
        pad8,
        GPUPowerOffGating,
        pad9,
        VDD_SYSReset,
        pad10[7],
        Cluster0CPUPowerSwitch[4],
        Cluster1CPUPowerSwitch[4],
        SuperStandbyFlag,
        CPUSoftwareEntry,
        SuperStandbySoftwareEntry,
        pad11[13],
        NMIIRQControl,
        NMIIRQEnable,
        NMIIRQStatus,
        pad12[5],
        PLL_AUDIOControl,
        PLL_AUDIOBias,
        PLL_AUDIOPatternControl,
        AUDIO_PLLControlSwitch,
        pad13[8],
        R_PIOHoldControl,
        OSC24MControl;
};
