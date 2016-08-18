#include <AllwinnerRSB.hpp>

struct AXP803 {
    static const struct Instance {
        Natural16 hardwareAddress;
        Natural8 runTimeAddress;
    } instances[];

    static void initialize() {
        auto RSB = AllwinnerRSB::instances[0].address;
        RSB->deviceAddress.hardwareAddress = instances[0].hardwareAddress;
        RSB->deviceAddress.runTimeAddress = instances[0].runTimeAddress;
        RSB->setRunTimeAddress();

        Natural8 type;
        if(!RSB->read(0x3, type))
            return;
        if(type != 0x51) {
            puts("[FAIL] AXP803");
            return;
        }

        // Natural16 voltage = (level < 32) ? 800+level*10 : 1120+(level-32)*20;
        const Natural8 level = 51; // 1500 mV
        if(RSB->write(0x24, level)) // DRAM_VCC (DCDC5)
            puts("[ OK ] AXP803");
    }
};
