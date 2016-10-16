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
    eth0->setMACAddress({{ 0x36, 0xC9, 0xE3, 0xF1, 0xB8, 0x05 }}); // 36:C9:E3:F1:B8:05

    Tcp::connection = new(reinterpret_cast<Natural8*>(eth0)-sizeof(Tcp::Connection))Tcp::Connection;
    Tcp::connection->receiveBuffer = reinterpret_cast<Natural8*>(dram);
    Tcp::connection->localPort = 1337;
    Tcp::connection->listen();

    while(Tcp::connection->status != Tcp::Connection::Finished) {
        Clock::update();
        eth0->poll();
        Tcp::poll();
    }

    puts("[ OK ] TCP download");
}
