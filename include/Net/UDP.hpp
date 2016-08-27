#include "TCP.hpp"

struct UDP {
    static constexpr Natural8 protocolID = 17;

    struct Packet {
        Natural16 sourcePort,
                  destinationPort,
                  length,
                  checksum;
        Natural8 payload[0];
    };

    static void received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* udpPacket);
};

void UDP::received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, UDP::Packet* udpPacket) {
    puts("UDP");
}
