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
        if(type != 0x51)
            puts("[FAIL] AXP803");
        else
            puts("[ OK ] AXP803");
    }

    static void configureDCDC5() {
        auto RSB = AllwinnerRSB::instances[0].address;

        // Natural16 voltage = (level < 32) ? 800+level*10 : 1120+(level-32)*20;
        Natural8 value = 51; // 1500 mV
        if(!RSB->write(0x24, value)) // DRAM_VCC (DCDC5)
            return;
    }

    static void configureDC1SW() {
        auto RSB = AllwinnerRSB::instances[0].address;

        Natural8 value = (1<<7); // DC1SW (Ethernet PHY)
        if(!RSB->write(0x12, value)) // Output power on-off control 2
            return;
    }
};
