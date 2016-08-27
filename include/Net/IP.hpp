#include "MAC.hpp"

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

        Address& operator=(const Address& other) {
            memcpy(this, &other, sizeof(other));
            return *this;
        }

        bool operator==(const Address& other) {
            return memcmp(this, &other, sizeof(other)) == 0;
        }

        bool operator!=(const Address& other) {
            return memcmp(this, &other, sizeof(other)) != 0;
        }
    };

    struct Packet {
        Natural8 internetHeaderLength : 4,
                 version : 4;
        Natural8 typeOfService;
        Natural16 totalLength,
                  identification;
        union {
            struct {
                Natural16 fragmentOffset : 13,
                          flags : 3;
            };
            Natural16 swap0;
        };
        Natural8 timeToLive,
                 protocol;
        Natural16 checksum;
        Address sourceAddress,
                destinationAddress;
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
            swapEndian(swap0);
            swapEndian(checksum);
        }

        void prepareTransmit(Natural16 length) {
            version = 4;
            internetHeaderLength = 5;
            typeOfService = 0;
            totalLength = length+internetHeaderLength*4;
            identification = 0;
            fragmentOffset = 0;
            flags = 0;
            timeToLive = 255;
            checksum = 0;
            checksum = headerChecksum();
            correctEndian();
        }
    };
    static_assert(sizeof(Packet) == 20);

    static void received(MAC::Frame* macFrame, Packet* packet);

    template<typename PayloadType>
    static void redirectToDriver(MAC::Frame* macFrame, Packet* packet) {
        if(packet->payloadChecksum<PayloadType>() == 0)
            PayloadType::received(
                macFrame,
                reinterpret_cast<IPvAnyPacket*>(packet),
                reinterpret_cast<typename PayloadType::Packet*>(packet->getPayload())
            );
        // else // TODO
    }
};

struct IPv6 {
    static constexpr Natural16 protocolID = 0x86DD;

    struct Address {
        Natural8 bytes[16];

        Address& operator=(const Address& other) {
            memcpy(this, &other, sizeof(other));
            return *this;
        }

        bool operator==(const Address& other) {
            return memcmp(this, &other, sizeof(other)) == 0;
        }

        bool operator!=(const Address& other) {
            return memcmp(this, &other, sizeof(other)) != 0;
        }
    };
    static constexpr Address localNetworkSegmentAllNodesMulticastAddress = {{ 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }};

    struct Packet {
        union {
            struct {
                Natural32 flowLabel : 20,
                          trafficClass : 8,
                          version : 4;
            };
            Natural32 swap0;
        };
        Natural16 payloadLength;
        Natural8 nextHeader,
                 hopLimit;
        Address sourceAddress,
                destinationAddress;
        Natural8 payload[0];

        template<typename PayloadType>
        Natural16 payloadChecksum() {
            struct {
                Natural32 length;
                Natural8 zeroPad[3], protocol;
            } pseudoHeader = {
                swapedEndian(payloadLength),
                { 0, 0, 0 }, PayloadType::protocolID
            };
            IPChecksumGenerator generator;
            generator.input(2*sizeof(Address), &sourceAddress);
            generator.input(sizeof(pseudoHeader), &pseudoHeader);
            generator.input(payloadLength, payload);
            return generator.output();
        }

        void correctEndian() {
            swapEndian(swap0);
            swapEndian(payloadLength);
        }

        void prepareTransmit(Natural16 length) {
            version = 6;
            trafficClass = 0;
            flowLabel = 0;
            payloadLength = length;
            hopLimit = 255;
            correctEndian();
        }
    };
    static_assert(sizeof(Packet) == 40);

    static void received(MAC::Frame* macFrame, Packet* packet);

    template<typename PayloadType>
    static void redirectToDriver(MAC::Frame* macFrame, Packet* packet) {
        if(packet->payloadChecksum<PayloadType>() == 0)
            PayloadType::received(
                macFrame,
                reinterpret_cast<IPvAnyPacket*>(packet),
                reinterpret_cast<typename PayloadType::Packet*>(packet->payload)
            );
        // else // TODO
    }

    static bool addressFromMACAddress(Address& dst, const MAC::Address& src) {
        if(src.bytes[0] == 0x33 && src.bytes[1] == 0x33) { // Multicast Addresses
            dst.bytes[0] = 0xFF;
            for(Natural8 i = 1; i < 12; ++i)
                dst.bytes[i] = 0x00;
            for(Natural8 i = 2; i < 6; ++i)
                dst.bytes[i+10] = src.bytes[i];
            return true;
        }
        if(!(src.bytes[0]&1)) { // Unicast Addresses
            dst.bytes[0] = 0xFE;
            dst.bytes[1] = 0x80;
            for(Natural8 i = 2; i < 8; ++i)
                dst.bytes[i] = 0x00;
            dst.bytes[8] = src.bytes[0]^0x02;
            dst.bytes[9] = src.bytes[1];
            dst.bytes[10] = src.bytes[2];
            dst.bytes[11] = 0xFF;
            dst.bytes[12] = 0xFE;
            dst.bytes[13] = src.bytes[3];
            dst.bytes[14] = src.bytes[4];
            dst.bytes[15] = src.bytes[5];
            return true;
        }
        return false;
    }

    static bool addressToMACAddress(MAC::Address& dst, const Address& src) {
        if(src.bytes[0] == 0xFF) { // Multicast Addresses
            dst.bytes[0] = 0x33;
            dst.bytes[1] = 0x33;
            for(Natural8 i = 2; i < 6; ++i)
                dst.bytes[i] = src.bytes[i+10];
            return true;
        }
        if(src.bytes[0] == 0xFE && src.bytes[2] == 0x80 && src.bytes[11] == 0xFF && src.bytes[12] == 0xFE) { // Unicast Addresses
            for(Natural8 i = 2; i < 8; ++i)
                if(src.bytes[i] != 0x00)
                    return false;
            dst.bytes[0] = src.bytes[8]^0x02;
            dst.bytes[1] = src.bytes[9];
            dst.bytes[2] = src.bytes[10];
            dst.bytes[3] = src.bytes[13];
            dst.bytes[4] = src.bytes[14];
            dst.bytes[5] = src.bytes[15];
            return true;
        }
        return false;
    }
};

union IPvAnyPacket {
    IPv4::Packet v4;
    IPv6::Packet v6;
};
