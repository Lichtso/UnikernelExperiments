#include <Net/Net.hpp>

void puts(const char* str) {
    auto uart = AllwinnerUART::instances[0].address;
    uart->puts(str);
    uart->putc('\r');
    uart->putc('\n');
}

void main() {

    auto ccu = AllwinnerCCU::instances[0].address;
    ccu->configureHSTimer();
    Clock::initialize();

    auto uart = AllwinnerUART::instances[0].address;
    ccu->configureUART0();
    uart->initialize();

    auto rsb = AllwinnerRSB::instances[0].address;
    ccu->configureRSB();
    rsb->initialize();
    AXP803::initialize();

    AXP803::configureDCDC5();
    ccu->configurePLL();
    ccu->configureDRAM();
    auto dram = AllwinnerDRAM::instances[0].address;
    dram->initialize();

    AXP803::configureDC1SW();
    ccu->configureEMAC();

    auto eth0 = new(dram)AllwinnerEMACDriver;
    eth0->initialize();
    Icmpv6::NeighborAdvertisement::transmit(eth0);

    while(1)
        eth0->poll();
}
