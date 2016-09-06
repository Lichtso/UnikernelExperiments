#include "Tcp.hpp"

struct Udp {
    static constexpr Natural8 protocolID = 17;

    struct Packet {
        Natural16 sourcePort,
                  destinationPort,
                  length,
                  checksum;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(sourcePort);
            swapEndian(destinationPort);
            swapEndian(length);
            swapEndian(checksum);
        }
    };

    static void received(Mac::Frame* macFrame, IpvAnyPacket* ipPacket, Packet* udpPacket);
};

void Udp::received(Mac::Frame* macFrame, IpvAnyPacket* ipPacket, Udp::Packet* udpPacket) {
    puts("UDP");
    udpPacket->correctEndian();
}
