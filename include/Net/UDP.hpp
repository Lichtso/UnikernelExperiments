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

    static void received(IPvAnyPacket* ipPacket, Packet* udpPacket);
};

void UDP::received(IPvAnyPacket* ipPacket, UDP::Packet* udpPacket) {
    puts("UDP");
}
