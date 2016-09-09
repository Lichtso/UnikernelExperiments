#include "Icmp.hpp"

struct Tcp {
    static constexpr Natural8 protocolID = 6;

    enum OptionType {
        End = 0,
        Padding = 1,
        MaximumSegmentSize = 2,
        WindowScale = 3,
        SelectiveAcknowledgementPermitted = 4,
        SelectiveAcknowledgement = 5,
        Timestamp = 8
    };

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

    struct Connection {
        Natural8 version;
        Natural16 localPort, remotePort;
        Ipv6::Address remoteAddress; // TODO: IpvAnyAddress

        // Status
        // Receive Buffer : DSACK
        // Transmit Buffer : Retransmission timer
        // Persist Timer
        // Congestion control, Slow Start, Congestion Avoidance, TCP-Reno
        // Maximum segment size
        // Timestamps
        Natural8 localWindowScale, remoteWindowScale;
        bool selectiveAcknowledgmentEnabled;
        // selectiveAcknowledgmentBuffer
    };

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Tcp::Packet* tcpPacket);
    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Tcp::Packet* tcpPacket);
};

void Tcp::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Tcp::Packet* tcpPacket) {
    puts("TCP");
    tcpPacket->correctEndian();
}

void Tcp::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Tcp::Packet* tcpPacket) {
    puts("TCP");
    tcpPacket->correctEndian();
}
