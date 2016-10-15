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
    #ifdef NETWORK_DEBUG
    puts("UDP");
    #endif
    udpPacket->correctEndian();
}

void Udp::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* udpPacket) {
    #ifdef NETWORK_DEBUG
    puts("UDP");
    #endif
    udpPacket->correctEndian();

    if(udpPacket->destinationPort != 3824)
        return;

    auto responseMacFrame = macInterface->createFrame(sizeof(Ipv6::Packet)+ipPacket->payloadLength);
    if(!macFrame)
        return;
    auto responseIpPacket = reinterpret_cast<Ipv6::Packet*>(responseMacFrame->payload);
    auto responseUdpPacket = reinterpret_cast<Packet*>(responseIpPacket->payload);
    responseUdpPacket->sourcePort = udpPacket->destinationPort;
    responseUdpPacket->destinationPort = udpPacket->sourcePort;
    memcpy(responseUdpPacket->payload, udpPacket->payload, udpPacket->totalLength-sizeof(Packet));
    responseIpPacket->destinationAddress = ipPacket->sourceAddress;
    responseIpPacket->sourceAddress = macInterface->ipv6LinkLocalAddress;
    responseUdpPacket->prepareTransmit(responseIpPacket, udpPacket->totalLength-sizeof(Packet));
    responseIpPacket->prepareTransmit();
    macInterface->transmitIpPacket(responseMacFrame);
}
