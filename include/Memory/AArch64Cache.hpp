#include <Hardware/AllwinnerA64.hpp>

namespace Cache {

enum Level {
    L1UnifiedCache = 0,
    L1DataCache = 0,
    L1InstructionCache = 1,
    L2UnifiedCache = 2,
    L2DataCache = 2,
    L2InstructionCache = 3,
    L3UnifiedCache = 4,
    L3DataCache = 4,
    L3InstructionCache = 5
};

void getSizesOfLevel(Level level, Natural32& setCountMinus1, Natural32& wayCountMinus1, Natural32& lineBytesLog2) {
    Natural32 cacheSizeIDRegister;
    asm volatile(
        "msr CSSELR_EL1, %x1\n"
        "isb\n"
        "mrs %x0, CCSIDR_EL1\n" : "=r"(cacheSizeIDRegister) : "r"(level)
    );
    setCountMinus1 = (cacheSizeIDRegister>>13)&0x7FFF;
    wayCountMinus1 = (cacheSizeIDRegister>>3)&0x3FF;
    lineBytesLog2 = (cacheSizeIDRegister&0x7)+4;
}

#define offsetCountDownLoop(index, maxIndex, offset, inner) { \
    index = maxIndex<<offset; \
    do { \
        inner \
        index -= 1<<offset; \
    } while(index > 0); \
}

void invalidateLevel(Level level) {
    if(level == L1InstructionCache)
        asm volatile("ic iallu\n");
    else if((level&1) == 0) {
        Natural32 wayIndex, wayMaxIndex, wayOffset, setIndex, setMaxIndex, setOffset;
        getSizesOfLevel(level, setMaxIndex, wayMaxIndex, setOffset);
        wayOffset = __builtin_clz(wayMaxIndex);
        offsetCountDownLoop(wayIndex, wayMaxIndex, wayOffset,
            offsetCountDownLoop(setIndex, setMaxIndex, setOffset,
                asm volatile("dc isw, %x0\n" : : "r"(wayIndex|setIndex|level));
            );
        );
        asm volatile("dsb sy\n");
    }
}

void invalidateTLB() {
    asm volatile("tlbi alle3\n");
}

void invalidateAll() {
    Natural32 cacheLevelIDRegister, level, levelEnd, cacheType;
    asm volatile("mrs %x0, CLIDR_EL1\n" : "=r"(cacheLevelIDRegister));
    levelEnd = (cacheLevelIDRegister>>23)&0x7;
    for(level = 0; level < levelEnd; ++level) {
        cacheType = (cacheLevelIDRegister>>(level/2*3))&0x7;
        if(cacheType >= 2)
            invalidateLevel(static_cast<Level>(level));
    }
    invalidateTLB();
}

void setActive(bool mmu, bool data, bool instruction) {
    asm volatile(
        "msr SCTLR_EL3, %x0\n"
        "isb\n" : : "r"(
        (mmu<<0)|
        (1<<1)| // Enable alignment fault check
        (data<<2)|
        (1<<3)| // Enable stack alignment check
        (instruction<<12)
    ));
}

void preFetch(void* address) {
    asm volatile("prfm pldl1keep, [%x0]\n" : : "r"(address));
}

void ensureRead(void* address) {
    asm volatile("dc ivac, %x0\n" : : "r"(address));
}

void ensureWrite(void* address) {
    asm volatile("dmb st\ndc cvac, %x0\n" : : "r"(address));
}

};
