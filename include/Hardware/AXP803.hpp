#include "AllwinnerRSB.hpp"

struct AXP803 {
    static const struct Instance {
        Natural16 hardwareAddress;
        Natural8 runTimeAddress;
    } instances[];

    static void initialize() {
        auto rsb = AllwinnerRSB::instances[0].address;
        rsb->deviceAddress.hardwareAddress = instances[0].hardwareAddress;
        rsb->deviceAddress.runTimeAddress = instances[0].runTimeAddress;
        rsb->setRunTimeAddress();

        Natural8 type;
        if(!rsb->read(0x3, type))
            return;
        if(type != 0x51)
            puts("[FAIL] AXP803");
        else
            puts("[ OK ] AXP803");
    }

    static void configureDCDC5() {
        auto rsb = AllwinnerRSB::instances[0].address;

        Natural8 value = 51; // 1500 mV
        if(!rsb->write(0x24, value)) // DRAM_VCC (DCDC5)
            return;
    }

    static void configureDC1SW() {
        auto rsb = AllwinnerRSB::instances[0].address;

        Natural8 value = (1<<7); // DC1SW (Ethernet PHY)
        if(!rsb->write(0x12, value)) // Output power on-off control 2
            return;
    }
};
