#include <Memory/AArch64Cache.hpp>

namespace MMU {
    enum AttributeIndex {
        Device = 0,
        NonCacheable = 1,
        WriteThrough = 2,
        WriteBack = 3
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
        Granule4 = 9,
        Granule16 = 11,
        Granule64 = 13
    };

    template<Natural16 addressBits>
    struct PageTable {
        PageTableEntry entries[2<<addressBits];
    };
};
