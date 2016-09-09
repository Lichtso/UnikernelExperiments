#include "Tcp.hpp"

struct Udp {
    static constexpr Natural8 protocolID = 17;

    struct Packet {
        Natural16 sourcePort,
                  destinationPort,
                  totalLength,
                  checksum;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(sourcePort);
            swapEndian(destinationPort);
            swapEndian(totalLength);
        }

        void prepareTransmit(Ipv4::Packet* ipPacket, Natural16 payloadLength) {
            totalLength = payloadLength+sizeof(Packet);
            ipPacket->protocol = protocolID;
            ipPacket->totalLength = totalLength;
            correctEndian();
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Udp>();
        }

        void prepareTransmit(Ipv6::Packet* ipPacket, Natural16 payloadLength) {
            totalLength = payloadLength+sizeof(Packet);
            ipPacket->nextHeader = protocolID;
            ipPacket->payloadLength = totalLength;
            correctEndian();
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Udp>();
        }
    };

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Packet* udpPacket);
    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* udpPacket);
};

void Udp::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Packet* udpPacket) {
    puts("UDP");
    udpPacket->correctEndian();
}

void Udp::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* udpPacket) {
    puts("UDP");
    udpPacket->correctEndian();
}
