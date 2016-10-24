#include "AllwinnerA64.hpp"

namespace AArch64 {

enum CacheLevel {
    L1DataCache = 0,
    L1InstructionCache = 1,
    L2UnifiedCache = 2
};

void invalidateCache(CacheLevel cacheLevel) {
    if(cacheLevel == L1InstructionCache)
        asm volatile("ic iallu\n");
    else {
        Natural32 cacheSizeIDRegister,
            setIndex, setIndexMax, setOffset,
            wayIndex, wayIndexMax, wayOffset;
        asm volatile(
            "msr CSSELR_EL1, %x1\n"
            "isb\n"
            "mrs %x0, CCSIDR_EL1\n" : "=r"(cacheSizeIDRegister) : "r"(cacheLevel)
        );
        setIndexMax = (cacheSizeIDRegister>>13)&0x7FFF, // S : NumSets : 127 / 511
        setOffset = (cacheSizeIDRegister&0x7)+4, // L : LineSize : 6 / 6
        wayIndexMax = (cacheSizeIDRegister>>3)&0x3FF, // 32-A : Associativity : 3 / 15
        wayOffset = __builtin_clz(wayIndexMax); // 30 / 28
        for(wayIndex = wayIndexMax<<wayOffset; wayIndex > 0; wayIndex -= 1<<wayOffset)
            for(setIndex = setIndexMax<<setOffset; setIndex > 0; setIndex -= 1<<setOffset)
                asm volatile("dc isw, %x0\n" : : "r"(wayIndex|setIndex|cacheLevel));
        asm volatile("dsb sy\n");
    }
}

void activateInstructionCache() {
    Natural32 systemControlRegisterEL3;
    asm volatile("mrs %x0, SCTLR_EL3\n" : "=r"(systemControlRegisterEL3));
    systemControlRegisterEL3 |= 1<<12;
    asm volatile("msr SCTLR_EL3, %x0\n" : : "r"(systemControlRegisterEL3));
}

};
