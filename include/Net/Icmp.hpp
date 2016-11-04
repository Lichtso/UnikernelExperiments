#include <Net/Ip.hpp>

#define IcmpReceivedCase(PayloadType) \
    case PayloadType::type: \
        reinterpret_cast<PayloadType*>(icmpPacket->payload)->received(macInterface, macFrame, ipPacket, icmpPacket); \
        break;

struct Icmpv4 {
    static constexpr Natural8 protocolID = 1;

    struct Packet {
        Natural8 type, code;
        Natural16 checksum;
        Natural8 payload[0];

        void prepareTransmit(Ipv4::Packet* ipPacket, Natural16 payloadLength) {
            ipPacket->protocol = protocolID;
            ipPacket->totalLength = sizeof(Packet)+payloadLength;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Icmpv4>();
        }
    };

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Packet* icmpPacket);
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

        void prepareTransmit(Ipv6::Packet* ipPacket, Natural16 payloadLength) {
            ipPacket->nextHeader = protocolID;
            ipPacket->payloadLength = sizeof(Packet)+payloadLength;
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Icmpv6>();
        }
    };

    struct DestinationUnreachable {
        static constexpr Natural8 type = 1;
        enum Code {
            NoRouteToDestination = 0,
            CommunicationProhibited = 1,
            BeyondScopeOfSourceAddress = 2,
            AddressUnreachable = 3,
            PortUnreachable = 4,
            SourceAddressFailedPolicy = 5,
            RejectRouteToDestination = 6
        };
        Natural32 pad0;

        void correctEndian() { }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("DestinationUnreachable");
            #endif
            correctEndian();
        }
    };

    struct PacketTooBig {
        static constexpr Natural8 type = 2;
        Natural32 maximumTransmissionUnit;

        void correctEndian() {
            swapEndian(maximumTransmissionUnit);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("PacketTooBig");
            #endif
            correctEndian();
        }
    };

    struct TimeExceeded {
        static constexpr Natural8 type = 3;
        enum Code {
            HopLimitExceeded = 0,
            FragmentReassembly = 1
        };
        Natural32 pad0;

        void correctEndian() { }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("TimeExceeded");
            #endif
            correctEndian();
        }
    };

    struct ParameterProblem {
        static constexpr Natural8 type = 4;
        enum Code {
            ErroneousHeaderField = 0,
            UnrecognizedNextHeaderType = 1,
            UnrecognizedIPv6Option = 2,
        };
        Natural32 pointer;

        void correctEndian() {
            swapEndian(pointer);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("ParameterProblem");
            #endif
            correctEndian();
        }
    };

    struct EchoRequest {
        static constexpr Natural8 type = 128;
        Natural16 identifier, sequenceNumber;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(identifier);
            swapEndian(sequenceNumber);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("EchoRequest");
            #endif
            correctEndian();

            EchoReply::transmit(macInterface, ipPacket);
        }
    };

    struct EchoReply {
        static constexpr Natural8 type = 129;
        Natural16 identifier, sequenceNumber;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(identifier);
            swapEndian(sequenceNumber);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("EchoReply");
            #endif
            correctEndian();
        }

        static bool transmit(Mac::Interface* macInterface, Ipv6::Packet* receivedIpPacket) {
            auto macFrame = macInterface->createFrame(sizeof(Ipv6::Packet)+receivedIpPacket->payloadLength);
            if(!macFrame)
                return false;
            auto ipPacket = reinterpret_cast<Ipv6::Packet*>(macFrame->payload);
            auto icmpPacket = reinterpret_cast<Packet*>(ipPacket->payload);
            auto echoReply = reinterpret_cast<EchoReply*>(icmpPacket->payload);
            auto echoRequest = reinterpret_cast<EchoRequest*>(reinterpret_cast<Packet*>(receivedIpPacket->payload)->payload);
            echoReply->identifier = echoRequest->identifier;
            echoReply->sequenceNumber = echoRequest->sequenceNumber;
            echoReply->correctEndian();
            ipPacket->destinationAddress = receivedIpPacket->sourceAddress;
            ipPacket->sourceAddress = macInterface->ipv6LinkLocalAddress;
            icmpPacket->type = type;
            icmpPacket->code = 0;
            icmpPacket->prepareTransmit(ipPacket, receivedIpPacket->payloadLength-sizeof(Packet));
            ipPacket->prepareTransmit();
            macInterface->transmitIpPacket(macFrame);
            return true;
        }
    };

    struct MulticastListenerQuery {
        static constexpr Natural8 type = 130;
        Natural16 maximumResponseCode, pad0;
        Ipv6::Address multicastAddress;
        Natural8 queriersRobustnessVariable : 3,
                 suppressRouterSideProcessing : 1,
                 pad1 : 4;
        Natural8 queriersQueryIntervalCode;
        Natural16 numberOfSources;
        Ipv6::Address sourceAddress[0];

        void correctEndian() {
            swapEndian(maximumResponseCode);
            swapEndian(numberOfSources);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("MulticastListenerQuery");
            #endif
            correctEndian();
        }
    };

    struct NeighborSolicitation {
        static constexpr Natural8 type = 135;
        Natural32 pad0;
        Ipv6::Address targetAddress;
        Option options[0];

        void correctEndian() { }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("NeighborSolicitation");
            #endif
            correctEndian();

            if(targetAddress != macInterface->ipv6LinkLocalAddress)
                return;
            macInterface->addNeighbor(macFrame->sourceAddress, ipPacket->sourceAddress);
            NeighborAdvertisement::transmit(macInterface, ipPacket);
        }
    };

    struct NeighborAdvertisement {
        static constexpr Natural8 type = 136;
        Natural32 pad0 : 5,
                  overrideFlag : 1,
                  solicitedFlag : 1,
                  routerFlag : 1,
                  pad1 : 24;
        Ipv6::Address targetAddress;
        Option options[0];

        void correctEndian() { }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("NeighborAdvertisement");
            #endif
            correctEndian();
        }

        static bool transmit(Mac::Interface* macInterface, Ipv6::Packet* receivedIpPacket = nullptr) {
            auto macFrame = macInterface->createFrame(sizeof(Ipv6::Packet)+sizeof(Packet)+sizeof(NeighborAdvertisement)+8);
            if(!macFrame)
                return false;
            auto ipPacket = reinterpret_cast<Ipv6::Packet*>(macFrame->payload);
            auto icmpPacket = reinterpret_cast<Packet*>(ipPacket->payload);
            auto neighborAdvertisement = reinterpret_cast<NeighborAdvertisement*>(icmpPacket->payload);
            neighborAdvertisement->routerFlag = 0;
            neighborAdvertisement->solicitedFlag = (receivedIpPacket) ? 1 : 0;
            neighborAdvertisement->overrideFlag = 1;
            neighborAdvertisement->pad0 = 0;
            neighborAdvertisement->pad1 = 0;
            neighborAdvertisement->targetAddress = (receivedIpPacket)
                ? reinterpret_cast<NeighborSolicitation*>(reinterpret_cast<Packet*>(receivedIpPacket->payload)->payload)->targetAddress
                : macInterface->ipv6LinkLocalAddress;
            neighborAdvertisement->options[0].type = Option::TargetLinkLayerAddress;
            neighborAdvertisement->options[0].chunks = 1;
            macInterface->getMACAddress(reinterpret_cast<Mac::Address&>(neighborAdvertisement->options[0].payload));
            neighborAdvertisement->correctEndian();
            ipPacket->destinationAddress = (receivedIpPacket) ? receivedIpPacket->sourceAddress : Ipv6::localNetworkSegmentAllNodesMulticastAddress;
            ipPacket->sourceAddress = macInterface->ipv6LinkLocalAddress;
            icmpPacket->type = type;
            icmpPacket->code = 0;
            icmpPacket->prepareTransmit(ipPacket, sizeof(NeighborAdvertisement)+8);
            ipPacket->prepareTransmit();
            macInterface->transmitIpPacket(macFrame);
            return true;
        }
    };

    struct MulticastListenerReport {
        static constexpr Natural8 type = 143;
        Natural16 pad0, numberOfRecords;
        struct MulticastListenerRecord {
            enum Type {
                MODE_IS_INCLUDE = 1,
                MODE_IS_EXCLUDE = 2,
                CHANGE_TO_INCLUDE_MODE = 3,
                CHANGE_TO_EXCLUDE_MODE = 4,
                ALLOW_NEW_SOURCES = 5,
                BLOCK_OLD_SOURCES = 6
            };
            Natural8 type, auxDataLength;
            Natural16 numberOfSources;
            Ipv6::Address multicastAddress, sourceAddress[0];
        } records[0];

        void correctEndian() {
            swapEndian(numberOfRecords);
            for(Natural16 i = 0; i < numberOfRecords; ++i)
                swapEndian(records[i].numberOfSources);
        }

        void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket) {
            #ifdef NETWORK_DEBUG
            puts("MulticastListenerReport");
            #endif
            correctEndian();
        }
    };

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Packet* icmpPacket);
};

void Icmpv4::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Icmpv4::Packet* icmpPacket) {
    #ifdef NETWORK_DEBUG
    puts("ICMPv4");
    #endif
}

void Icmpv6::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Icmpv6::Packet* icmpPacket) {
    #ifdef NETWORK_DEBUG
    auto uart = AllwinnerUART::instances[0].address;
    puts("ICMPv6");
    #endif

    switch(icmpPacket->type) {
        // IcmpReceivedCase(DestinationUnreachable)
        // IcmpReceivedCase(PacketTooBig)
        // IcmpReceivedCase(TimeExceeded)
        // IcmpReceivedCase(ParameterProblem)
        IcmpReceivedCase(EchoRequest)
        // IcmpReceivedCase(EchoReply)
        IcmpReceivedCase(MulticastListenerQuery)
        IcmpReceivedCase(NeighborSolicitation)
        // IcmpReceivedCase(NeighborAdvertisement)
        // IcmpReceivedCase(MulticastListenerReport)
        default:
            #ifdef NETWORK_DEBUG
            uart->putDec(icmpPacket->type);
            puts(" unknown type");
            #endif
            break;
    }
}
