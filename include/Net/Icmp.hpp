#include "Ip.hpp"

struct Icmpv4 {
    static constexpr Natural8 protocolID = 1;

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        Natural8 payload[0];

        template<typename PacketType>
        void prepareTransmit(Ipv4::Packet* ipPacket, Natural16 payloadLength = sizeof(PacketType)) {
            ipPacket->protocol = protocolID;
            ipPacket->totalLength = sizeof(Packet)+payloadLength;
            reinterpret_cast<PacketType*>(payload)->correctEndian();
            type = PacketType::type;
            code = PacketType::code;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Icmpv4>();
        }
    };

    static void received(Mac::Frame* macFrame, IpPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(Mac::Frame* macFrame, IpPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(macFrame, &ipPacket->v4, icmpPacket);
    }
};

struct Icmpv6 {
    static constexpr Natural8 protocolID = 58;

    struct Option {
        enum Type {
            SourceLinkLayerAddress = 1,
            TargetLinkLayerAddress = 2,
            PrefixInformation = 3,
            RedirectedHeader = 4,
            MaximumTransmissionUnit = 5
        };
        Natural8 type, chunks, payload[0];
    };

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        Natural8 payload[0];

        template<typename PacketType>
        void prepareTransmit(Ipv6::Packet* ipPacket, Natural16 payloadLength = 0) {
            ipPacket->nextHeader = protocolID;
            ipPacket->payloadLength = sizeof(Packet)+sizeof(PacketType)+payloadLength;
            reinterpret_cast<PacketType*>(payload)->correctEndian();
            type = PacketType::type;
            code = PacketType::code;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Icmpv6>();
        }
    };

    struct EchoRequest {
        static constexpr Natural8 type = 128, code = 0;
        Natural16 identifier, sequenceNumber;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(identifier);
            swapEndian(sequenceNumber);
        }

        void received(Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("EchoRequest");
        }
    };

    struct EchoReply {
        static constexpr Natural8 type = 129, code = 0;
        Natural16 identifier, sequenceNumber;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(identifier);
            swapEndian(sequenceNumber);
        }

        void received(Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("EchoReply");
        }
    };

    struct NeighborSolicitation {
        static constexpr Natural8 type = 135, code = 0;
        Natural32 pad0;
        Ipv6::Address targetAddress;
        Option options[0];

        void correctEndian() { }

        void received(Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("NeighborSolicitation");
        }
    };

    struct NeighborAdvertisement {
        static constexpr Natural8 type = 136, code = 0;
        Natural32 pad0 : 5,
                  overrideFlag : 1,
                  solicitedFlag : 1,
                  routerFlag : 1,
                  pad1 : 24;
        Ipv6::Address targetAddress;
        Option options[0];

        void correctEndian() { }

        void received(Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("NeighborAdvertisement");
        }
    };

    static void received(Mac::Frame* macFrame, IpPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(Mac::Frame* macFrame, IpPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(macFrame, &ipPacket->v6, icmpPacket);
    }
};

void Icmpv4::received(Mac::Frame* macFrame, IpPacket* ipPacket, Icmpv4::Packet* icmpPacket) {
    puts("ICMPv4");
}

void Icmpv6::received(Mac::Frame* macFrame, IpPacket* ipPacket, Icmpv6::Packet* icmpPacket) {
    puts("ICMPv6");

    auto UART = AllwinnerUART::instances[0].address;
    switch(icmpPacket->type) {
        case Icmpv6::EchoRequest::type:
            redirectToDriver<Icmpv6::EchoRequest>(macFrame, ipPacket, icmpPacket);
            break;
        case Icmpv6::EchoReply::type:
            redirectToDriver<Icmpv6::EchoReply>(macFrame, ipPacket, icmpPacket);
            break;
        case Icmpv6::NeighborSolicitation::type:
            redirectToDriver<Icmpv6::NeighborSolicitation>(macFrame, ipPacket, icmpPacket);
            break;
        case Icmpv6::NeighborAdvertisement::type:
            redirectToDriver<Icmpv6::NeighborAdvertisement>(macFrame, ipPacket, icmpPacket);
            break;
        default:
            UART->putHex(icmpPacket->type);
            puts(" unknown type");
            break;
    }
}
