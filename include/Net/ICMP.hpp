#include "IP.hpp"

struct ICMPv4 {
    static constexpr Natural8 protocolID = 1;

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        union {
            Natural8 payload[0];
        };

        void correctEndian() {
            swapEndian(checksum);
        }

        template<typename PacketType>
        void prepareTransmit(IPv4::Packet* ipPacket) {
            ipPacket->protocol = protocolID;
            reinterpret_cast<PacketType*>(payload)->correctEndian();
            type = PacketType::type;
            code = PacketType::code;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<ICMPv4>();
            correctEndian();
        }
    };

    static void received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(macFrame, &ipPacket->v4, icmpPacket);
    }
};

struct ICMPv6 {
    static constexpr Natural8 protocolID = 58;

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(checksum);
        }

        template<typename PacketType>
        void prepareTransmit(IPv6::Packet* ipPacket) {
            ipPacket->nextHeader = protocolID;
            reinterpret_cast<PacketType*>(payload)->correctEndian();
            type = PacketType::type;
            code = PacketType::code;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<ICMPv6>();
            correctEndian();
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

        void received(MAC::Frame* macFrame, IPv6::Packet* ipPacket, Packet* icmpPacket) {
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

        void received(MAC::Frame* macFrame, IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("EchoReply");
        }
    };

    struct NeighborSolicitation {
        static constexpr Natural8 type = 135, code = 0;
        Natural32 pad0;
        IPv6::Address targetAddress;

        void correctEndian() { }

        void received(MAC::Frame* macFrame, IPv6::Packet* ipPacket, Packet* icmpPacket) {
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
        IPv6::Address targetAddress;

        void correctEndian() { }

        void received(MAC::Frame* macFrame, IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("NeighborAdvertisement");
        }
    };

    static void received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(macFrame, &ipPacket->v6, icmpPacket);
    }
};

void ICMPv4::received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, ICMPv4::Packet* icmpPacket) {
    puts("ICMPv4");
}

void ICMPv6::received(MAC::Frame* macFrame, IPvAnyPacket* ipPacket, ICMPv6::Packet* icmpPacket) {
    icmpPacket->correctEndian();
    puts("ICMPv6");

    auto UART = AllwinnerUART::instances[0].address;
    switch(icmpPacket->type) {
        case ICMPv6::EchoRequest::type:
            redirectToDriver<ICMPv6::EchoRequest>(macFrame, ipPacket, icmpPacket);
            break;
        case ICMPv6::EchoReply::type:
            redirectToDriver<ICMPv6::EchoReply>(macFrame, ipPacket, icmpPacket);
            break;
        case ICMPv6::NeighborSolicitation::type:
            redirectToDriver<ICMPv6::NeighborSolicitation>(macFrame, ipPacket, icmpPacket);
            break;
        case ICMPv6::NeighborAdvertisement::type:
            redirectToDriver<ICMPv6::NeighborAdvertisement>(macFrame, ipPacket, icmpPacket);
            break;
        default:
            UART->putHex(icmpPacket->type);
            puts(" unknown type");
            break;
    }
}
