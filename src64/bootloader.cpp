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

    auto dramEnd = reinterpret_cast<NativeNaturalType>(dram)*3;
    auto eth0 = new(reinterpret_cast<Natural8*>(dramEnd)-sizeof(AllwinnerEMACDriver))AllwinnerEMACDriver;
    eth0->initialize();
    Icmpv6::NeighborAdvertisement::transmit(eth0);

    Tcp::connection = new(reinterpret_cast<Natural8*>(eth0)-sizeof(Tcp::Connection))Tcp::Connection;
    Tcp::connection->receiveBuffer = reinterpret_cast<Natural8*>(dram);
    Tcp::connection->localPort = 1337;
    Tcp::connection->listen();

    while(1) {
        Clock::update();
        eth0->poll();
        Tcp::poll();
    }
}
