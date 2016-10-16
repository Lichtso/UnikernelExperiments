#include "Udp.hpp"

void Mac::Interface::linkStatusChanged() {
    if(linkStatus)
        Icmpv6::NeighborAdvertisement::transmit(this);
    else
        invalidateNeighborCache();
}

struct AllwinnerEMACDriver : public Mac::Interface {
    static const Natural16 transmitBufferCount = 128, receiveBufferCount = 128, bufferSize = 1536;

    AllwinnerEMAC::TransmitDescriptor transmitDescriptorRing[transmitBufferCount];
    AllwinnerEMAC::ReceiveDescriptor receiveDescriptorRing[receiveBufferCount];
    struct {
        Natural8 pad0[bufferSize];
    } transmitBuffers[transmitBufferCount], receiveBuffers[receiveBufferCount];
    struct AllwinnerEMAC::TransmitDescriptor *transmitedDescriptor, *transmitableDescriptor;
    struct AllwinnerEMAC::ReceiveDescriptor *receivedDescriptor;

    bool initialize() {
        Mac::Interface::initialize();

        auto EMAC = AllwinnerEMAC::instances[0].address;
        Clock::printUptime();
        EMAC->initialize();

        for(Natural8 i = 0; i < transmitBufferCount; ++i) {
            auto descriptor = &transmitDescriptorRing[i];
            descriptor->status.raw = 0;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = 0;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptEnable = 1;
            descriptor->bufferAddress = fromPointer(&transmitBuffers[i])+2;
            descriptor->next = fromPointer(&transmitDescriptorRing[(i+1)%transmitBufferCount]);
        }
        for(Natural8 i = 0; i < receiveBufferCount; ++i) {
            auto descriptor = &receiveDescriptorRing[i];
            descriptor->status.raw = 0;
            descriptor->status.DMAOwnership = 1;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = bufferSize;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptDisable = 0;
            descriptor->bufferAddress = fromPointer(&receiveBuffers[i])+2;
            descriptor->next = fromPointer(&receiveDescriptorRing[(i+1)%receiveBufferCount]);
        }
        transmitedDescriptor = transmitableDescriptor = &transmitDescriptorRing[0];
        receivedDescriptor = &receiveDescriptorRing[0];
        EMAC->transmitDMA = fromPointer(transmitableDescriptor);
        EMAC->receiveDMA = fromPointer(receivedDescriptor);
        EMAC->enableReceiver(true);
        Clock::printUptime();
        return true;
    }

    bool poll() {
        auto uart = AllwinnerUART::instances[0].address;
        auto EMAC = AllwinnerEMAC::instances[0].address;
        bool nextLinkStatus = EMAC->link();
        if(linkStatus != nextLinkStatus) {
            linkStatus = nextLinkStatus;
            Clock::printUptime();
            if(linkStatus) {
                uart->puts("[ OK ] ");
                uart->putDec(EMAC->linkSpeed());
                puts(" Mbit/s ethernet link");
            } else
                puts("[FAIL] Lost ethernet link");
            linkStatusChanged();
        }
        Natural32 length = 0, left = transmitBufferCount;
        AllwinnerEMAC::TransmitDescriptor* transmitPeekDescriptor = transmitedDescriptor;
        while(1) {
            if(transmitPeekDescriptor->status.DMAOwnership) // Pending Frame
                break;
            else if(transmitPeekDescriptor->control.bufferSize == 0) { // Vacant Frame
                EMAC->enableTransmitter(false);
                break;
            }
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
                --left;
                if(left == 0)
                    break;
                length = 0;
            }
            transmitPeekDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitPeekDescriptor->next);
        }
        if(EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitSuspended)
            EMAC->enableTransmitter(false);
        length = 0;
        left = receiveBufferCount/2;
        AllwinnerEMAC::ReceiveDescriptor* receivePeekDescriptor = receivedDescriptor;
        while(1) {
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
                --left;
                if(left == 0)
                    break;
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

    bool transmit(Mac::Frame* macFrame) {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        /*auto uart = AllwinnerUART::instances[0].address;
        for(Natural16 j = 0; j < 150; ++j)
            uart->putHex(reinterpret_cast<Natural8*>(macFrame)[j]);
        puts(" macFrame");*/

        auto index = (fromPointer(macFrame)-2-fromPointer(transmitBuffers))/bufferSize;
        auto descriptor = &transmitDescriptorRing[index];
        descriptor->status.DMAOwnership = 1;

        /* TODO: Holes in the transmit queue
        auto descriptorToClear = transmitedDescriptor;
        while(descriptorToClear != descriptor) {
            uart->putHex(reinterpret_cast<Natural64>(descriptorToClear));
            puts(" descriptorToClear");
            if(descriptorToClear->status.DMAOwnership == 0) {
                descriptorToClear->control.bufferSize = 0;
                descriptorToClear->status.raw = 0;
                descriptorToClear->status.DMAOwnership = 1;
            }
            descriptorToClear = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(descriptorToClear->next);
        }*/
        if(EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitStopped ||
           EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitSuspended)
            EMAC->enableTransmitter(true);
        return true;
    }

    void setMACAddress(const Mac::Address& src) {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->setMACAddress(0, &src);
        IpAddress::macToIpv6(ipv6LinkLocalAddress, src);
    }

    void getMACAddress(Mac::Address& dst) {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->getMACAddress(0, &dst);
    }

    void transmited(Natural32 errors, Natural32 length, Mac::Frame* macFrame) {
        auto uart = AllwinnerUART::instances[0].address;
        #ifdef NETWORK_DEBUG
        uart->putHex(reinterpret_cast<Natural64>(transmitedDescriptor));
        puts(" transmit success");
        #endif
        if(errors) {
            Clock::printUptime();
            uart->putHex(errors);
            puts(" transmit error\n");
            return;
        }
    }

    void received(Natural32 errors, Natural32 length, Mac::Frame* macFrame) {
        auto uart = AllwinnerUART::instances[0].address;
        #ifdef NETWORK_DEBUG
        Clock::printUptime();
        uart->putDec(length);
        puts(" receive success");
        /*for(Natural16 j = 0; j < 1500; ++j)
            uart->putHex(reinterpret_cast<Natural8*>(macFrame)[j]);
        puts(" macFrame");*/
        for(Natural16 j = 0; j < 6; ++j)
            uart->putHex(macFrame->destinationAddress.bytes[j]);
        puts(" destination MAC");
        for(Natural16 j = 0; j < 6; ++j)
            uart->putHex(macFrame->sourceAddress.bytes[j]);
        puts(" source MAC");
        #endif

        if(errors) {
            uart->putHex(errors);
            puts(" receive error");
            return;
        }

        macFrame->correctEndian();
        switch(macFrame->type) {
            case Ipv4::protocolID:
                Ipv4::received(this, macFrame, reinterpret_cast<Ipv4::Packet*>(macFrame->payload));
                break;
            case Ipv6::protocolID:
                Ipv6::received(this, macFrame, reinterpret_cast<Ipv6::Packet*>(macFrame->payload));
                break;
            default:
                #ifdef NETWORK_DEBUG
                uart->putHex(macFrame->type);
                puts(" unknown protocol");
                #endif
                break;
        }
        #ifdef NETWORK_DEBUG
        puts("");
        #endif
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
    #ifdef NETWORK_DEBUG
    auto uart = AllwinnerUART::instances[0].address;
    puts("IPv4");
    for(Natural16 j = 0; j < 4; ++j)
        uart->putHex(ipPacket->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 4; ++j)
        uart->putHex(ipPacket->sourceAddress.bytes[j]);
    puts(" source IP");
    #endif

    if(ipPacket->headerChecksum() != 0) {
        #ifdef NETWORK_DEBUG
        puts("Checksum error");
        #endif
        return;
    }
    ipPacket->correctEndian();
    switch(ipPacket->protocol) {
        Ipv4ReceivedCase(Icmpv4)
        Ipv4ReceivedCase(Tcp)
        Ipv4ReceivedCase(Udp)
        default:
            #ifdef NETWORK_DEBUG
            uart->putDec(ipPacket->protocol);
            puts(" unknown protocol");
            #endif
            break;
    }
}

void Ipv6::received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket) {
    #ifdef NETWORK_DEBUG
    auto uart = AllwinnerUART::instances[0].address;
    puts("IPv6");
    for(Natural16 j = 0; j < 16; ++j)
        uart->putHex(ipPacket->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 16; ++j)
        uart->putHex(ipPacket->sourceAddress.bytes[j]);
    puts(" source IP");
    #endif

    ipPacket->correctEndian();
    switch(ipPacket->nextHeader) {
        Ipv6ReceivedCase(Icmpv6)
        Ipv6ReceivedCase(Tcp)
        Ipv6ReceivedCase(Udp)
        default:
            #ifdef NETWORK_DEBUG
            uart->putDec(ipPacket->nextHeader);
            puts(" unknown protocol");
            #endif
            break;
    }
}
