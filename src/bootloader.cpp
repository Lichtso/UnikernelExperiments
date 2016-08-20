#include <AllwinnerA64.hpp>

void puts(const char* str) {
    auto UART = AllwinnerUART::instances[0].address;
    UART->puts(str);
    UART->putc('\r');
    UART->putc('\n');
}

void main() {
    auto CCU = AllwinnerCCU::instances[0].address;

    auto UART = AllwinnerUART::instances[0].address;
    CCU->configureUART0();
    UART->initialize();

    auto RSB = AllwinnerRSB::instances[0].address;
    CCU->configureRSB();
    RSB->initialize();
    AXP803::initialize();

    AXP803::configureDCDC5();
    CCU->configurePLL();
    CCU->configureDRAM();
    DRAM::initialize();

    auto EMAC = AllwinnerEMAC::instances[0].address;
    AXP803::configureDC1SW();
    CCU->configureEMAC();
}
