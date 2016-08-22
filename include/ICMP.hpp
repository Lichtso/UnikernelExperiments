#include <InternetProtocol.hpp>

struct ICMPv4 {
    static constexpr Natural8 protocolID = 1;

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        union {
            Natural8 payload[0];
        };
    };

    static void received(IPvAnyPacket* ipPacket, Packet* icmpPacket);
};

struct ICMPv6 {
    static constexpr Natural8 protocolID = 58;

    struct EchoRequest {
        static constexpr Natural8 type = 128, code = 0;
        Natural16 identifier, sequenceNumber;
    };

    struct EchoReply {
        static constexpr Natural8 type = 129, code = 0;
        Natural16 identifier, sequenceNumber;
    };

    struct NeighborSolicitation {
        static constexpr Natural8 type = 135, code = 0;
        Natural32 pad0;
        IPv6::Address targetAddress;
    };

    struct NeighborAdvertisement {
        static constexpr Natural8 type = 136, code = 0;
        Natural32 pad0 : 29,
                  overrideFlag : 1,
                  solicitedFlag : 1,
                  routerFlag : 1;
        IPv6::Address targetAddress;
    };

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        union {
            Natural8 payload[0];
            EchoRequest echoRequest;
            EchoReply echoReply;
            NeighborSolicitation neighborSolicitation;
            NeighborAdvertisement neighborAdvertisement;
        };

        void correctEndian() {
            swapEndian(checksum);
        }
    };

    static void received(IPvAnyPacket* ipPacket, Packet* icmpPacket);
};

void ICMPv4::received(IPvAnyPacket* ipPacket, ICMPv4::Packet* icmpPacket) {
    puts("ICMPv4");
}

void ICMPv6::received(IPvAnyPacket* ipPacket, ICMPv6::Packet* icmpPacket) {
    icmpPacket->correctEndian();
    puts("ICMPv6");

    auto UART = AllwinnerUART::instances[0].address;
    switch(icmpPacket->type) {
        case ICMPv6::EchoRequest::type:
            puts("EchoRequest");
            break;
        case ICMPv6::EchoReply::type:
            puts("EchoReply");
            break;
        case ICMPv6::NeighborSolicitation::type:
            puts("NeighborSolicitation");
            break;
        case ICMPv6::NeighborAdvertisement::type:
            puts("NeighborAdvertisement");
            break;
        default:
            UART->putHex(icmpPacket->type);
            puts(" unknown type");
            break;
    }
}
