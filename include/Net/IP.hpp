#include "Mac.hpp"

struct IpChecksumGenerator {
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

struct Ipv4 {
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
                          pad0 : 1,
                          doNotFragment : 1,
                          moreFragments : 1;
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
            IpChecksumGenerator generator;
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
            IpChecksumGenerator generator;
            generator.input(2*sizeof(Address), &sourceAddress);
            generator.input(sizeof(pseudoHeader), &pseudoHeader);
            generator.input(payloadLength, getPayload());
            return generator.output();
        }

        void correctEndian() {
            swapEndian(totalLength);
            swapEndian(identification);
            swapEndian(swap0);
        }

        void prepareTransmit() {
            version = 4;
            internetHeaderLength = 5;
            typeOfService = 0;
            totalLength += internetHeaderLength*4;
            identification = 0;
            fragmentOffset = 0;
            pad0 = 0;
            doNotFragment = 1;
            moreFragments = 0;
            timeToLive = 255;
            correctEndian();
            checksum = 0;
            checksum = headerChecksum();
        }
    };
    static_assert(sizeof(Packet) == 20);

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Packet* packet);
};

struct Ipv6 {
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
            IpChecksumGenerator generator;
            generator.input(2*sizeof(Address), &sourceAddress);
            generator.input(sizeof(pseudoHeader), &pseudoHeader);
            generator.input(payloadLength, payload);
            return generator.output();
        }

        void correctEndian() {
            swapEndian(swap0);
            swapEndian(payloadLength);
        }

        void prepareTransmit() {
            version = 6;
            trafficClass = 0;
            flowLabel = 0;
            hopLimit = 255;
            correctEndian();
        }
    };
    static_assert(sizeof(Packet) == 40);

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Packet* packet);
};

union IpPacket {
    Ipv4::Packet v4;
    Ipv6::Packet v6;
};

union IpAddress {
    Ipv4::Address v4;
    Ipv6::Address v6;

    static bool macToIpv4(Ipv4::Address& dst, const Mac::Address& src) {
        if(src.bytes[0] == 0x01 && src.bytes[1] == 0x00 && src.bytes[2] == 0x5E && !(src.bytes[3]&0x80)) { // Multicast Addresses
            dst.bytes[0] = 224;
            dst.bytes[1] = src.bytes[3];
            dst.bytes[2] = src.bytes[4];
            dst.bytes[3] = src.bytes[5];
            return true;
        }
        for(Natural8 i = 0; i < 6; ++i) // Broadcast Address
            if(src.bytes[i] != 0xFF)
                return false;
        for(Natural8 i = 0; i < 4; ++i)
            dst.bytes[i] = 255;
        return true;
    }

    static bool ipv4ToMac(Mac::Address& dst, const Ipv4::Address& src) {
        if(src.bytes[0] >= 224 && src.bytes[0] <= 239) { // Multicast Addresses
            dst.bytes[0] = 0x01;
            dst.bytes[1] = 0x00;
            dst.bytes[2] = 0x5E;
            dst.bytes[3] = src.bytes[1]&0x7F;
            dst.bytes[4] = src.bytes[2];
            dst.bytes[5] = src.bytes[3];
            return true;
        }
        for(Natural8 i = 0; i < 4; ++i) // Broadcast Address
            if(src.bytes[i] != 255)
                return false;
        for(Natural8 i = 0; i < 6; ++i)
            dst.bytes[i] = 0xFF;
        return true;
    }

    static void ipv4ToIpv6(Ipv6::Address& dst, const Ipv4::Address& src) {
        for(Natural8 i = 0; i < 10; ++i)
            dst.bytes[i] = 0x00;
        dst.bytes[10] = 0xFF;
        dst.bytes[11] = 0xFF;
        for(Natural8 i = 0; i < 4; ++i)
            dst.bytes[i+12] = src.bytes[i];
    }

    static void ipv6ToIpv4(Ipv4::Address& dst, const Ipv6::Address& src) {
        for(Natural8 i = 0; i < 4; ++i)
            dst.bytes[i] = src.bytes[i+12];
    }

    static bool macToIpv6(Ipv6::Address& dst, const Mac::Address& src) {
        if(src.bytes[0] == 0x33 && src.bytes[1] == 0x33) { // Multicast Addresses
            dst.bytes[0] = 0xFF;
            for(Natural8 i = 1; i < 12; ++i)
                dst.bytes[i] = 0x00;
            for(Natural8 i = 2; i < 6; ++i)
                dst.bytes[i+10] = src.bytes[i];
            return true;
        }
        if(!(src.bytes[0]&1)) { // EUI-64
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

    static bool ipv6ToMac(Mac::Address& dst, const Ipv6::Address& src) {
        // TODO: Lookup addess scheme
        if(src.bytes[0] == 0xFF && (src.bytes[1]&0xF0) == 0x00) { // Multicast Addresses
            dst.bytes[0] = 0x33;
            dst.bytes[1] = 0x33;
            for(Natural8 i = 2; i < 6; ++i)
                dst.bytes[i] = src.bytes[i+10];
            return true;
        }
        if(src.bytes[0] == 0xFE && src.bytes[1] == 0x80) { // LinkLocal Address
            for(Natural8 i = 2; i < 8; ++i)
                if(src.bytes[i] != 0x00)
                    return false;
            if(src.bytes[11] != 0xFF || src.bytes[12] != 0xFE) // EUI-64
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

struct Mac::Interface {
    static const Natural8 neighborCacheEntryCount = 8;
    // Ipv4::Address ipv4LinkLocalAddress;
    Ipv6::Address ipv6LinkLocalAddress;
    struct NeighborCacheEntry {
        Ipv6::Address ipv6Address;
        Mac::Address macAddress;
        Natural16 usageCounter;
    } neighborCache[8];
    bool linkStatus;
    // TODO: Tables
    // Multicast Listener (Report)
    // Destination Cache
    // Default Router List
    // Prefix List
    // MTU (IPv6 Packet Total Length): minimum is 1280 Bytes

    virtual bool initialize() {
        linkStatus = false;
        linkStatusChanged();
        return true;
    }
    virtual bool poll() = 0;
    virtual Frame* createFrame(Natural16 payloadLength) = 0;
    virtual bool transmit(Mac::Frame* frame) = 0;

    virtual void setMACAddress(const Address& src) = 0;
    virtual void getMACAddress(Address& dst) = 0;

    void invalidateNeighbor(NeighborCacheEntry* entry) {
        entry->usageCounter = 0;
        memset(&entry->ipv6Address, 0, sizeof(Ipv6::Address));
        memset(&entry->macAddress, 0, sizeof(Mac::Address));
    }

    void invalidateNeighborCache() {
        for(NativeNaturalType i = 0; i < neighborCacheEntryCount; ++i)
            invalidateNeighbor(&neighborCache[i]);
    }

    NeighborCacheEntry* findNeighbor(const Ipv6::Address& ipv6Address) {
        NeighborCacheEntry* entry = nullptr;
        for(NativeNaturalType i = 0; i < neighborCacheEntryCount; ++i) {
            // neighborCache[i].usageCounter *= 0.99; // TODO: Reduce usageCounter
            if(neighborCache[i].usageCounter > 0 && neighborCache[i].ipv6Address == ipv6Address) {
                ++neighborCache[i].usageCounter;
                entry = &neighborCache[i];
            }
        }
        return entry;
    }

    void addNeighbor(const Mac::Address& macAddress, const Ipv6::Address& ipv6Address) {
        NeighborCacheEntry* entry = findNeighbor(ipv6Address);
        if(entry) {
            entry->macAddress = macAddress;
            return;
        }
        entry = &neighborCache[0];
        for(NativeNaturalType i = 1; i < neighborCacheEntryCount; ++i)
            if(neighborCache[i].usageCounter < entry->usageCounter)
                entry = &neighborCache[i];
        entry->ipv6Address = ipv6Address;
        entry->macAddress = macAddress;
        entry->usageCounter = 1;
    }

    void prepareTransmit(Mac::Frame* macFrame) {
        getMACAddress(macFrame->sourceAddress);
        macFrame->correctEndian();
    }

    bool transmitIpPacket(Mac::Frame* macFrame) {
        auto uart = AllwinnerUART::instances[0].address;
        switch(macFrame->payload[0]>>4) {
            case 4: {
                // auto ipv4Packet = reinterpret_cast<Ipv4::Packet*>(macFrame->payload);
                macFrame->type = Ipv4::protocolID;
                // TODO
            } return false;
            case 6: {
                auto ipv6Packet = reinterpret_cast<Ipv6::Packet*>(macFrame->payload);
                macFrame->type = Ipv6::protocolID;
                NeighborCacheEntry* entry = findNeighbor(ipv6Packet->destinationAddress);
                if(entry)
                    macFrame->destinationAddress = entry->macAddress;
                else if(!IpAddress::ipv6ToMac(macFrame->destinationAddress, ipv6Packet->destinationAddress)) {
                    for(Natural16 j = 0; j < 16; ++j)
                        uart->putHex(ipv6Packet->destinationAddress.bytes[j]);
                    puts(" Could not resolve mac address");
                    return false;
                }
            } break;
            default:
                uart->putHex(macFrame->type);
                puts(" unknown protocol");
                return false;
        }
        prepareTransmit(macFrame);
        return transmit(macFrame);
    }

    void linkStatusChanged();
};
