#include <MAC.hpp>

struct IPChecksumGenerator {
    Natural32 accumulator = 0;

    void input(Natural32 length, void* typelessBuffer) {
        auto buffer = reinterpret_cast<Natural8*>(typelessBuffer);
        if(length&1)
            accumulator += buffer[--length];
        auto pos = reinterpret_cast<Natural16*>(buffer);
        auto bufferEnd = reinterpret_cast<Natural16*>(buffer+length);
        while(pos != bufferEnd)
            accumulator += *(pos++);
    }

    Natural16 output() {
        while(accumulator>>16)
            accumulator = (accumulator>>16)+(accumulator&0xFFFF);
        return ~static_cast<Natural16>(accumulator);
    }
};

union IPvAnyPacket;

struct IPv4 {
    static constexpr Natural16 protocolID = 0x0800;

    struct Address {
        Natural8 bytes[4];
    };

    struct Packet {
        Natural16 internetHeaderLength : 4,
                  version : 4;
        Natural8  typeOfService;
        Natural16 totalLength,
                  identification;
        Natural16 fragmentOffset : 13,
                  flags : 3;
        Natural8  timeToLive,
                  protocol;
        Natural16 checksum;
        Address sourceAddress, destinationAddress;
        Natural8 data[0];

        Natural16 headerChecksum() {
            IPChecksumGenerator generator;
            generator.input(internetHeaderLength*4, this);
            return generator.output();
        }

        Natural16 getPayloadLength() {
            return totalLength-internetHeaderLength*4;
        }

        Natural8* getPayload() {
            return &data[internetHeaderLength*4-20];
        }

        template<typename PayloadHeader>
        Natural16 payloadChecksum() {
            Natural16 payloadLength = getPayloadLength();
            struct {
                Natural8 zeroPad, protocol;
                Natural16 length;
            } pseudoHeader = {
                0, PayloadHeader::protocolID,
                swapedEndian(payloadLength)
            };
            IPChecksumGenerator generator;
            generator.input(2*sizeof(Address), &sourceAddress);
            generator.input(sizeof(pseudoHeader), &pseudoHeader);
            generator.input(payloadLength, getPayload());
            return generator.output();
        }

        void correctEndian() {
            swapEndian(totalLength);
            swapEndian(identification);
            swapEndian(checksum);
        }
    };
    static_assert(sizeof(Packet) == 20);

    static void received(Packet* packet);

    template<typename PayloadProtocol>
    static void handle(Packet* packet) {
        if(packet->payloadChecksum<PayloadProtocol>() == 0)
            PayloadProtocol::received(
                reinterpret_cast<IPvAnyPacket*>(packet),
                reinterpret_cast<typename PayloadProtocol::Packet*>(packet->getPayload())
            );
        // else // TODO
    }
};

struct IPv6 {
    static constexpr Natural16 protocolID = 0x86DD;

    struct Address {
        Natural8 bytes[16];
    };

    struct Packet {
        Natural32 flowLabel : 20,
                  trafficClass : 8,
                  version : 4;
        Natural16 payloadLength;
        Natural8  nextHeader,
                  hopLimit;
        Address sourceAddress, destinationAddress;
        Natural8 payload[0];

        template<typename PayloadProtocol>
        Natural16 payloadChecksum() {
            struct {
                Natural32 length;
                Natural8 zeroPad[3], protocol;
            } pseudoHeader = {
                swapedEndian(payloadLength),
                { 0, 0, 0 }, PayloadProtocol::protocolID
            };
            IPChecksumGenerator generator;
            generator.input(2*sizeof(Address), &sourceAddress);
            generator.input(sizeof(pseudoHeader), &pseudoHeader);
            generator.input(payloadLength, payload);
            return generator.output();
        }

        void correctEndian() {
            swapEndian(payloadLength);
        }
    };
    static_assert(sizeof(Packet) == 40);

    static void received(Packet* packet);

    template<typename PayloadProtocol>
    static void handle(Packet* packet) {
        if(packet->payloadChecksum<PayloadProtocol>() == 0)
            PayloadProtocol::received(
                reinterpret_cast<IPvAnyPacket*>(packet),
                reinterpret_cast<typename PayloadProtocol::Packet*>(packet->payload)
            );
        // else // TODO
    }
};

union IPvAnyPacket {
    IPv4::Packet v4;
    IPv6::Packet v6;
};
