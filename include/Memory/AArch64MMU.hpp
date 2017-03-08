#include <Memory/AArch64Cache.hpp>

namespace MMU {
    const Natural64 attributePalette = 0x0000FFBB440C0400;

    enum AttributeIndex {
        DeviceNGNRNE = 0,
        DeviceNGNRE = 1,
        DeviceGRE = 2,
        NonCacheable = 3,
        WriteThrough = 4, // Outer and inner, read-write-allocate
        WriteBack = 5 // Outer and inner, read-write-allocate
    };

    enum Shareability {
        NonShareable = 0,
        OuterShareable = 2,
        InnerShareable = 3
    };

    union PageTableEntry {
        struct {
            Natural64 valid : 1,
                      nextLevelEnabled : 1,
                      lastLevelAttributeIndex : 3,
                      lastLevelNonSecure : 1,
                      lastLevelAccessPermissions : 2,
                      lastLevelShareability : 2,
                      lastLevelAccessFlag, : 1,
                      lastLevelNotGlobal : 1,
                      address : 36,
                      pad0 : 4,
                      lastLevelContiguous : 1,
                      lastLevelPrivilegedExecuteNever : 1,
                      lastLevelExecuteNever : 1,
                      lastLevelSoftwareDefined : 4,
                      nextLevelPrivilegedExecuteNever : 1,
                      nextLevelExecuteNever : 1,
                      nextLevelAccessPermissions : 2,
                      nextLevelNonSecure : 1;
        };
        Natural64 raw;
    };

    enum Granularity {
        Granule4KiB = 9,
        Granule16KiB = 11,
        Granule64KiB = 13
    };

    template<Natural16 addressBits>
    struct PageTable {
        PageTableEntry entries[2<<addressBits];
    };

    volatile Natural8* initialize() {
        auto dram = AllwinnerDRAM::instances[0].address;
        auto dramCom = AllwinnerDRAMCOM::instances[0].address;
        auto dramEnd = reinterpret_cast<NativeNaturalType>(dram)+(1ULL<<dramCom->getDRAMSize());
        auto level2Table = reinterpret_cast<volatile PageTable<Granule64KiB>*>(dramEnd-sizeof(PageTable<Granule64KiB>));

        for(Natural32 index = 0; index < 8192; ++index) {
            auto entry = &level2Table->entries[index];
            entry->raw = 0x20000000*index;
            entry->lastLevelAccessFlag = 1;
        }

        auto entry = &level2Table->entries[0];
        entry->valid = 1;
        entry->lastLevelAttributeIndex = DeviceNGNRE;
        entry->lastLevelShareability = OuterShareable;

        for(Natural32 index = 2; index < 6; ++index) {
            entry = &level2Table->entries[index];
            entry->valid = 1;
            entry->lastLevelAttributeIndex = WriteBack;
            entry->lastLevelShareability = InnerShareable;
        }

        asm volatile(
            "dmb st\n"
            "msr TTBR0_EL3, %x0\n"
            "msr MAIR_EL3, %x1\n"
            "msr TCR_EL3, %x2\n"
            "isb\n" : : "r"(level2Table), "r"(attributePalette), "r"(
            (64-32)| // Starting at level 2
            (3<< 8)| // Inner cacheability: Normal memory, Inner Write-Back no Write-Allocate Cacheable
            (3<<10)| // Outer cacheability: Normal memory, Outer Write-Back no Write-Allocate Cacheable
            (3<<12)| // Shareability: Inner shareable
            (1<<14)| // Granule size: 64 KiB
            (0<<16)| // Physical address space: 4 GiB
            (0<<20)  // Top byte used in the address calculation
        ));

        return reinterpret_cast<volatile Natural8*>(level2Table);
    }
};
