#include <AllwinnerA64.hpp>

void puts(const char* str) {
    auto UART = AllwinnerUART::instances[0].address;
    UART->puts(str);
    UART->putc('\r');
    UART->putc('\n');
}

void main() {
    auto CCU = AllwinnerCCU::instances[0].address;
    CCU->configureUART0();

    auto PIO = AllwinnerPIO::instances[0].address;
    PIO->banks[1].configure[1].slot0 = 4; // PB8 : UART0_TX
    PIO->banks[1].configure[1].slot1 = 4; // PB9 : UART0_RX
    PIO->banks[1].pull[0].slot9 = 1; // PB9 : PullUP

    auto UART = AllwinnerUART::instances[0].address;
    UART->initialize();

    auto RSB = AllwinnerRSB::instances[0].address;
    RSB->initialize();
    AXP803::initialize();

    CCU->configurePLL();
    CCU->configureDRAM();
    DRAM::initialize();
}
