#include "Udp.hpp"

struct AllwinnerEMACDriver : public Mac::Interface {
    static const Natural16 transmitBufferCount = 128, receiveBufferCount = 128, bufferSize = 1536;

    AllwinnerEMAC::TransmitDescriptor transmitDescriptorRing[transmitBufferCount];
    AllwinnerEMAC::ReceiveDescriptor receiveDescriptorRing[receiveBufferCount];
    struct {
        Natural8 pad0[bufferSize];
    } transmitBuffers[transmitBufferCount], receiveBuffers[receiveBufferCount];
    struct AllwinnerEMAC::TransmitDescriptor *transmitedDescriptor, *transmitableDescriptor;
    struct AllwinnerEMAC::ReceiveDescriptor *receivedDescriptor;
    bool linkStatus;

    bool initialize() {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        Clock::printUptime();
        EMAC->initialize();
        Clock::printUptime();
        EMAC->waitForLink();
        linkStatus = true;
        Clock::printUptime();

        Mac::Address macAddress = {{ 0x36, 0xC9, 0xE3, 0xF1, 0xB8, 0x05 }}; // 36:C9:E3:F1:B8:05
        setMACAddress(macAddress);
        IpAddress::macToIpv6(ipv6LinkLocalAddress, macAddress);

        for(Natural8 i = 0; i < transmitBufferCount; ++i) {
            auto descriptor = &transmitDescriptorRing[i];
            descriptor->bufferAddress = fromPointer(&transmitBuffers[i])+2;
            descriptor->next = fromPointer(&transmitDescriptorRing[(i+1)%transmitBufferCount]);
            descriptor->status.raw = 0;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = 0;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptEnable = 1;
        }
        for(Natural8 i = 0; i < receiveBufferCount; ++i) {
            auto descriptor = &receiveDescriptorRing[i];
            descriptor->bufferAddress = fromPointer(&receiveBuffers[i])+2;
            descriptor->next = fromPointer(&receiveDescriptorRing[(i+1)%receiveBufferCount]);
            descriptor->status.raw = 0;
            descriptor->status.DMAOwnership = 1;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = bufferSize;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptDisable = 0;
        }
        transmitedDescriptor = transmitableDescriptor = &transmitDescriptorRing[0];
        receivedDescriptor = &receiveDescriptorRing[0];
        EMAC->transmitDMA = fromPointer(transmitableDescriptor);
        EMAC->receiveDMA = fromPointer(receivedDescriptor);
        EMAC->enableReceiver(true);
        return true;
    }

    bool poll() {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        bool nextLinkStatus = EMAC->link();
        if(linkStatus != nextLinkStatus)
            linkStatusChanged();
        linkStatus = nextLinkStatus;
        Natural32 length = 0;
        AllwinnerEMAC::TransmitDescriptor* transmitPeekDescriptor = transmitedDescriptor;
        while(1) {
            if(transmitPeekDescriptor->control.bufferSize == 0) { // Vacant Frame
                EMAC->enableTransmitter(false);
                break;
            }
            if(transmitPeekDescriptor->status.DMAOwnership) // Pending Frame
                break;
            length += transmitPeekDescriptor->control.bufferSize;
            if(transmitPeekDescriptor->control.last) {
                transmited(
                    transmitPeekDescriptor->status.raw&AllwinnerEMAC::TransmitDescriptor::errorMask,
                    length,
                    reinterpret_cast<Mac::Frame*>(transmitedDescriptor->bufferAddress));
                while(1) {
                    transmitedDescriptor->status.raw = 0;
                    transmitedDescriptor->control.bufferSize = 0;
                    bool last = (transmitedDescriptor == transmitPeekDescriptor);
                    transmitedDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitedDescriptor->next);
                    if(last)
                        break;
                }
                length = 0;
            }
            transmitPeekDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitPeekDescriptor->next);
        }
        if(EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitSuspended)
            EMAC->enableTransmitter(false);
        length = 0;
        AllwinnerEMAC::ReceiveDescriptor* receivePeekDescriptor = receivedDescriptor;
        while(1) { // TODO: Max count
            if(receivePeekDescriptor->status.DMAOwnership) // Vacant Frame
                break;
            length += receivePeekDescriptor->control.bufferSize;
            if(receivePeekDescriptor->status.last) {
                received(
                    receivePeekDescriptor->status.raw&AllwinnerEMAC::ReceiveDescriptor::errorMask,
                    length,
                    reinterpret_cast<Mac::Frame*>(receivedDescriptor->bufferAddress));
                while(1) {
                    receivedDescriptor->status.DMAOwnership = 1;
                    bool last = (receivedDescriptor == receivePeekDescriptor);
                    receivedDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivedDescriptor->next);
                    if(last)
                        break;
                }
                length = 0;
            }
            receivePeekDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivePeekDescriptor->next);
        }
        if(EMAC->receiveDMAStatus.status == AllwinnerEMAC::ReceiveStopped ||
           EMAC->receiveDMAStatus.status == AllwinnerEMAC::ReceiveSuspended) {
            EMAC->receiveDMA = fromPointer(receivedDescriptor);
            EMAC->enableReceiver(true);
        }
        return true;
    }

    Mac::Frame* createFrame(Natural16 payloadLength) {
        if(!linkStatus)
            return nullptr;
        auto descriptor = transmitableDescriptor;
        Natural16 totalLength = sizeof(Mac::Frame)+payloadLength, remainingLength = totalLength;
        while(1) {
            if(descriptor->control.bufferSize > 0)
                return nullptr;
            auto fragmentLength = min(remainingLength, bufferSize);
            remainingLength -= fragmentLength;
            if(remainingLength == 0)
                break;
            descriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(descriptor->next);
        }
        descriptor = transmitableDescriptor;
        transmitableDescriptor->control.first = true;
        remainingLength = totalLength;
        while(1) {
            transmitableDescriptor->control.bufferSize = min(remainingLength, bufferSize);
            remainingLength -= transmitableDescriptor->control.bufferSize;
            bool last = (remainingLength == 0);
            transmitableDescriptor->control.last = last;
            transmitableDescriptor->status.DMAOwnership = !transmitableDescriptor->control.first;
            transmitableDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitableDescriptor->next);
            if(last)
                break;
            transmitableDescriptor->control.first = false;
        }
        return reinterpret_cast<Mac::Frame*>(descriptor->bufferAddress);
    }

    bool transmit(Mac::Frame* frame) {
        getMACAddress(frame->sourceAddress);
        frame->correctEndian();

        auto uart = AllwinnerUART::instances[0].address;
        for(Natural16 j = 0; j < 128; ++j)
            uart->putHex(reinterpret_cast<Natural8*>(frame)[j]);
        puts(" frame");

        auto index = (fromPointer(frame)-2-fromPointer(transmitBuffers))/bufferSize;
        auto descriptor = &transmitDescriptorRing[index];
        descriptor->status.DMAOwnership = 1;
        auto EMAC = AllwinnerEMAC::instances[0].address;
        if(EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitStopped ||
           EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitSuspended)
            EMAC->enableTransmitter(true);
        return true;
    }

    void setMACAddress(const Mac::Address& src) {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->setMACAddress(0, &src);
    }

    void getMACAddress(Mac::Address& dst) {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->getMACAddress(0, &dst);
    }

    void linkStatusChanged() {
        if(linkStatus)
            puts("Lost connection");
        else
            puts("Acquired connection");
    }

    void transmited(Natural32 errors, Natural32 length, Mac::Frame* frame) {
        Clock::printUptime();
        auto uart = AllwinnerUART::instances[0].address;
        if(errors) {
            uart->putHex(errors);
            puts(" transmit errors\n");
            return;
        }
        uart->putHex(length);
        puts(" transmit success\n");
    }

    void received(Natural32 errors, Natural32 length, Mac::Frame* frame) {
        Clock::printUptime();
        auto uart = AllwinnerUART::instances[0].address;
        for(Natural16 j = 0; j < 1500; ++j)
            uart->putHex(reinterpret_cast<Natural8*>(frame)[j]);
        puts(" frame");

        frame->correctEndian();

        if(errors) {
            uart->putHex(errors);
            puts(" receive error");
            return;
        }
        uart->putHex(length);
        puts(" receive success");

        for(Natural16 j = 0; j < 6; ++j)
            uart->putHex(frame->destinationAddress.bytes[j]);
        puts(" destination MAC");
        for(Natural16 j = 0; j < 6; ++j)
            uart->putHex(frame->sourceAddress.bytes[j]);
        puts(" source MAC");

        switch(frame->type) {
            case Ipv4::protocolID:
                Ipv4::received(this, frame, reinterpret_cast<Ipv4::Packet*>(frame->payload));
                break;
            case Ipv6::protocolID:
                Ipv6::received(this, frame, reinterpret_cast<Ipv6::Packet*>(frame->payload));
                break;
            default:
                uart->putHex(frame->type);
                puts(" unknown protocol");
                break;
        }
        puts("");
    }
};

#define Ipv4ReceivedCase(PayloadType) \
    case PayloadType::protocolID: \
        if(ipPacket->payloadChecksum<PayloadType>() == 0) \
            PayloadType::received(macInterface, macFrame, ipPacket, reinterpret_cast<typename PayloadType::Packet*>(ipPacket->getPayload())); \
        /* else TODO */ \
        break;

#define Ipv6ReceivedCase(PayloadType) \
    case PayloadType::protocolID: \
        if(ipPacket->payloadChecksum<PayloadType>() == 0) \
            PayloadType::received(macInterface, macFrame, ipPacket, reinterpret_cast<typename PayloadType::Packet*>(ipPacket->payload)); \
        /* else TODO */ \
        break;

void Ipv4::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket) {
    puts("IPv4");

    auto uart = AllwinnerUART::instances[0].address;
    if(ipPacket->headerChecksum() != 0) {
        puts("Checksum error");
        return;
    }
    ipPacket->correctEndian();

    for(Natural16 j = 0; j < 4; ++j)
        uart->putHex(ipPacket->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 4; ++j)
        uart->putHex(ipPacket->sourceAddress.bytes[j]);
    puts(" source IP");

    switch(ipPacket->protocol) {
        Ipv4ReceivedCase(Icmpv4)
        Ipv4ReceivedCase(Tcp)
        Ipv4ReceivedCase(Udp)
        default:
            uart->putHex(ipPacket->protocol);
            puts(" unknown protocol");
            break;
    }
}

void Ipv6::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket) {
    puts("IPv6");
    ipPacket->correctEndian();

    auto uart = AllwinnerUART::instances[0].address;
    for(Natural16 j = 0; j < 16; ++j)
        uart->putHex(ipPacket->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 16; ++j)
        uart->putHex(ipPacket->sourceAddress.bytes[j]);
    puts(" source IP");

    switch(ipPacket->nextHeader) {
        Ipv6ReceivedCase(Icmpv6)
        Ipv6ReceivedCase(Tcp)
        Ipv6ReceivedCase(Udp)
        default:
            uart->putHex(ipPacket->nextHeader);
            puts(" unknown protocol");
            break;
    }
}
