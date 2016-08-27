#include "ICMP.hpp"

struct TCP {
    static constexpr Natural8 protocolID = 6;

    struct Packet {
        Natural16 sourcePort,
                  destinationPort;
        Natural32 sequenceNumber,
                  acknowledgmentNumber;
        Natural16 dataOffset : 4,
                  pad0 : 3,
                  ns : 1,
                  congestionWindowReduced : 1,
                  ece : 1,
                  urgent : 1,
                  acknowledgment : 1,
                  push : 1,
                  reset : 1,
                  synchronize : 1,
                  finish : 1;
        Natural16 windowSize,
                  checksum,
                  urgentPointer;
        Natural8 payload[0];
    };

    static void received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* tcpPacket);
};

void TCP::received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, TCP::Packet* tcpPacket) {
    puts("TCP");
}
