#include <Net/Net.hpp>

void puts(const char* str) {
    auto UART = AllwinnerUART::instances[0].address;
    UART->puts(str);
    UART->putc('\r');
    UART->putc('\n');
}

void main() {
    auto CCU = AllwinnerCCU::instances[0].address;
    auto HSTimer = AllwinnerHSTimer::instances[0].address;
    CCU->configureHSTimer();
    HSTimer->initialize();

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

    auto eth0 = new(DRAM::instances[0].address)AllwinnerEMACDriver;
    AXP803::configureDC1SW();
    CCU->configureEMAC();
    eth0->initialize();
    Icmpv6::NeighborAdvertisement::transmit(eth0);

    while(1)
        eth0->poll();
}
