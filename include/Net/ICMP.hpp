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

    static void received(IPvAnyPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(IPvAnyPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(&ipPacket->v4, icmpPacket);
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

        void received(IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("EchoRequest");

            // TODO: Experimental
            auto frame = reinterpret_cast<MAC::Frame*>(reinterpret_cast<Natural32>(ipPacket)-sizeof(MAC::Frame));
            auto MAC = reinterpret_cast<struct MAC*>(DRAM::instances[0].address);
            MAC::Frame* responseFrame = MAC->prepareTransmit(sizeof(MAC::Frame)+sizeof(IPv6::Packet)+ipPacket->payloadLength);
            if(responseFrame) {
                auto responseIpPacket = reinterpret_cast<IPv6::Packet*>(responseFrame->payload);
                auto responseIcmpPacket = reinterpret_cast<ICMPv6::Packet*>(responseIpPacket->payload);

                auto UART = AllwinnerUART::instances[0].address;
                UART->putHex(reinterpret_cast<Natural32>(responseFrame));
                puts(" responseFrame");
                UART->putHex(reinterpret_cast<Natural32>(responseIpPacket));
                puts(" responseIpPacket");
                UART->putHex(reinterpret_cast<Natural32>(responseIcmpPacket));
                puts(" responseIcmpPacket");
                UART->putHex(ipPacket->payloadLength-8);
                puts(" echo length");

                memcpy(responseIcmpPacket->payload, this, ipPacket->payloadLength);
                memcpy(&responseFrame->destinationAddress, &frame->sourceAddress, sizeof(MAC::Address));
                auto EMAC = AllwinnerEMAC::instances[0].address;
                EMAC->getMACAddress(0, &responseFrame->sourceAddress);
                memcpy(&responseIpPacket->destinationAddress, &ipPacket->sourceAddress, sizeof(IPv6::Address));
                IPv6::addressFromMACAddress(&responseIpPacket->sourceAddress, &responseFrame->sourceAddress);

                responseIcmpPacket->prepareTransmit<ICMPv6::EchoReply>(responseIpPacket);
                responseIpPacket->prepareTransmit(ipPacket->payloadLength);
                responseFrame->type = IPv6::protocolID;
                MAC->transmit(responseFrame);
            } else
                puts("prepareTransmit error");
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

        void received(IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("EchoReply");
        }
    };

    struct NeighborSolicitation {
        static constexpr Natural8 type = 135, code = 0;
        Natural32 pad0;
        IPv6::Address targetAddress;

        void correctEndian() { }

        void received(IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("NeighborSolicitation");
        }
    };

    struct NeighborAdvertisement {
        static constexpr Natural8 type = 136, code = 0;
        Natural32 pad0 : 29,
                  overrideFlag : 1,
                  solicitedFlag : 1,
                  routerFlag : 1;
        IPv6::Address targetAddress;

        void correctEndian() { }

        void received(IPv6::Packet* ipPacket, Packet* icmpPacket) {
            puts("NeighborAdvertisement");
        }
    };

    static void received(IPvAnyPacket* ipPacket, Packet* icmpPacket);

    template<typename PayloadType>
    static void redirectToDriver(IPvAnyPacket* ipPacket, Packet* icmpPacket) {
        auto packet = reinterpret_cast<PayloadType*>(icmpPacket->payload);
        packet->correctEndian();
        packet->received(&ipPacket->v6, icmpPacket);
    }
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
            redirectToDriver<ICMPv6::EchoRequest>(ipPacket, icmpPacket);
            break;
        case ICMPv6::EchoReply::type:
            redirectToDriver<ICMPv6::EchoReply>(ipPacket, icmpPacket);
            break;
        case ICMPv6::NeighborSolicitation::type:
            redirectToDriver<ICMPv6::NeighborSolicitation>(ipPacket, icmpPacket);
            break;
        case ICMPv6::NeighborAdvertisement::type:
            redirectToDriver<ICMPv6::NeighborAdvertisement>(ipPacket, icmpPacket);
            break;
        default:
            UART->putHex(icmpPacket->type);
            puts(" unknown type");
            break;
    }
}
