#include <Net/Net.hpp>

struct BootFileHeader {
    constexpr static Natural8  magicSeed[] = {'e', 'G', 'O', 'N', '.', 'B', 'T', '0'};
    constexpr static Natural32 checkSumSeed = 0x5F0A6C39,
                               blockSize = 512;

    Natural32 jumpInstruction;
    Natural8 magic[8];
    Natural32 checkSum, payloadLength;

    bool validate() {
        if(payloadLength == 0 || payloadLength%blockSize != 0)
            return false;
        if(memcmp(magic, magicSeed, sizeof(magic)) != 0)
            return false;
        Natural32 checkSumA = checkSum, checkSumB = 0;
        checkSum = checkSumSeed;
        for(Natural32 i = 0; i < payloadLength/4; ++i)
            checkSumB += reinterpret_cast<Natural32*>(this)[i];
        return checkSumA == checkSumB;
    }
};

void main() {
    auto ccu = AllwinnerCCU::instances[0].address;
    ccu->configureHSTimer();
    Clock::initialize();

    Cache::invalidateAll();
    Cache::setActive(false, false, true);

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
    auto pageTable = MMU::initialize();

    AXP803::configureDC1SW();
    ccu->configureEMAC();
    auto eth0 = new(pageTable-sizeof(AllwinnerEMACDriver))AllwinnerEMACDriver;
    eth0->initialize();
    eth0->setMACAddress({{ 0x36, 0xC9, 0xE3, 0xF1, 0xB8, 0x05 }});

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

    auto bootFileHeader = reinterpret_cast<BootFileHeader*>(Tcp::connection->receiveBuffer);
    if(bootFileHeader->validate()) {
        puts("[ OK ] Payload validation");
        reinterpret_cast<void(*)()>(bootFileHeader)();
        puts("[ OK ] Payload execution");
    } else
        puts("[FAIL] Payload validation");
}
