#include "AllwinnerSMHC.hpp"

const struct AllwinnerCPUCFG::Instance AllwinnerCPUCFG::instances[] = {
    { reinterpret_cast<AllwinnerCPUCFG*>(0x01700000) }
};

const struct AllwinnerSYSCTL::Instance AllwinnerSYSCTL::instances[] = {
    { reinterpret_cast<AllwinnerSYSCTL*>(0x01C00000) }
};

const struct AllwinnerSMHC::Instance AllwinnerSMHC::instances[] = {
    { reinterpret_cast<AllwinnerSMHC*>(0x01C0F000) },
    { reinterpret_cast<AllwinnerSMHC*>(0x01C10000) },
    { reinterpret_cast<AllwinnerSMHC*>(0x01C11000) }
};

const struct AllwinnerCCU::Instance AllwinnerCCU::instances[] = {
    { reinterpret_cast<AllwinnerCCU*>(0x01C20000) }
};

const struct AllwinnerTimer::Instance AllwinnerTimer::instances[] = {
    { reinterpret_cast<AllwinnerTimer*>(0x01C20C00), { 50, 51 } }
};

const struct AllwinnerUART::Instance AllwinnerUART::instances[] = {
    { reinterpret_cast<AllwinnerUART*>(0x01C28000), 32, 6 },
    { reinterpret_cast<AllwinnerUART*>(0x01C28400), 33, 7 },
    { reinterpret_cast<AllwinnerUART*>(0x01C28800), 34, 8 },
    { reinterpret_cast<AllwinnerUART*>(0x01C28C00), 35, 9 },
    { reinterpret_cast<AllwinnerUART*>(0x01C29000), 36, 10 }
};

const struct AllwinnerEMAC::Instance AllwinnerEMAC::instances[] = {
    { reinterpret_cast<AllwinnerEMAC*>(0x01C30000), 114 }
};

const struct AllwinnerHSTimer::Instance AllwinnerHSTimer::instances[] = {
    { reinterpret_cast<AllwinnerHSTimer*>(0x01C60000), 83 }
};

const struct AllwinnerDRAMCOM::Instance AllwinnerDRAMCOM::instances[] = {
    { reinterpret_cast<AllwinnerDRAMCOM*>(0x01C62000) }
};

const struct AllwinnerDRAMCTL::Instance AllwinnerDRAMCTL::instances[] = {
    { reinterpret_cast<AllwinnerDRAMCTL*>(0x01C63000) }
};

const struct GICPL400::Instance GICPL400::instances[] = {
    { reinterpret_cast<GICPL400*>(0x01C80000) }
};

const struct AllwinnerPIO::Instance AllwinnerPIO::instances[] = {
    { reinterpret_cast<AllwinnerPIO*>(0x01C20800) }, // CPUx-PORT (PIO)
    { reinterpret_cast<AllwinnerPIO*>(0x01F02C00) }  // CPUs-PORT (R_PIO)
};

const struct AllwinnerPRCM::Instance AllwinnerPRCM::instances[] = {
    { reinterpret_cast<AllwinnerPRCM*>(0x01F01400) }
};

const struct AllwinnerRSB::Instance AllwinnerRSB::instances[] = {
    { reinterpret_cast<AllwinnerRSB*>(0x01F03400), 71 }
};

const struct AXP803::Instance AXP803::instances[] = {
    { 0x3A3, 0x2D }
};

const struct AllwinnerDRAM::Instance AllwinnerDRAM::instances[] = {
    { reinterpret_cast<AllwinnerDRAM*>(0x40000000) }
};
