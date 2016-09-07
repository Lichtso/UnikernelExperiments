#include "Icmp.hpp"

struct Tcp {
    static constexpr Natural8 protocolID = 6;

    struct Packet {
        Natural16 sourcePort,
                  destinationPort;
        Natural32 sequenceNumber,
                  acknowledgmentNumber;
        Natural16 congestionWindowReduced : 1,
                  ece : 1,
                  urgent : 1,
                  acknowledgment : 1,
                  push : 1,
                  reset : 1,
                  synchronize : 1,
                  finish : 1,
                  dataOffset : 4,
                  pad0 : 3,
                  ns : 1;
        Natural16 windowSize,
                  checksum,
                  urgentPointer;
        Natural8 options[0];

        Natural8* getPayload() {
            return reinterpret_cast<Natural8*>(this)+dataOffset;
        }

        void correctEndian() {
            swapEndian(sourcePort);
            swapEndian(destinationPort);
            swapEndian(sequenceNumber);
            swapEndian(acknowledgmentNumber);
            swapEndian(windowSize);
            swapEndian(checksum);
            swapEndian(urgentPointer);
        }
    };

    static void received(Mac::Frame* macFrame, IpPacket* ipPacket, Tcp::Packet* tcpPacket);
};

void Tcp::received(Mac::Frame* macFrame, IpPacket* ipPacket, Tcp::Packet* tcpPacket) {
    puts("TCP");
    tcpPacket->correctEndian();
}
