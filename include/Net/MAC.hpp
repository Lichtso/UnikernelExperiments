#include "../Hardware/AllwinnerA64.hpp"

struct MAC {
    static const Natural16 transmitBufferCount = 128, receiveBufferCount = 128, bufferSize = 1536;

    struct Address {
        Natural8 bytes[6];
    };

    struct Frame {
        Address destinationAddress, sourceAddress;
        Natural16 type;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(type);
        }
    };
    static_assert(sizeof(Frame) == 14);

    static void transmited(Natural32 errors, Natural32 length, Frame* frame);
    static void received(Natural32 errors, Natural32 length, Frame* frame);

    AllwinnerEMAC::TransmitDescriptor transmitDescriptorRing[transmitBufferCount];
    AllwinnerEMAC::ReceiveDescriptor receiveDescriptorRing[receiveBufferCount];
    struct {
        Natural8 pad0[bufferSize];
    } transmitBuffers[transmitBufferCount], receiveBuffers[receiveBufferCount];
    struct AllwinnerEMAC::TransmitDescriptor *transmitedDescriptor, *transmitableDescriptor;
    struct AllwinnerEMAC::ReceiveDescriptor *receivedDescriptor;

    void initialize() {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->initialize();
        for(Natural8 i = 0; i < transmitBufferCount; ++i) {
            auto descriptor = &transmitDescriptorRing[i];
            descriptor->bufferAddress = reinterpret_cast<Natural32>(&transmitBuffers[i])+2;
            descriptor->next = reinterpret_cast<Natural32>(&transmitDescriptorRing[(i+1)%transmitBufferCount]);
            descriptor->status.raw = 0;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = 0;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptEnable = 1;
        }
        for(Natural8 i = 0; i < receiveBufferCount; ++i) {
            auto descriptor = &receiveDescriptorRing[i];
            descriptor->bufferAddress = reinterpret_cast<Natural32>(&receiveBuffers[i])+2;
            descriptor->next = reinterpret_cast<Natural32>(&receiveDescriptorRing[(i+1)%receiveBufferCount]);
            descriptor->status.raw = 0;
            descriptor->status.DMAOwnership = 1;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = bufferSize;
            descriptor->control.chainMode = 1;
            descriptor->control.completionInterruptDisable = 0;
        }
        transmitedDescriptor = transmitableDescriptor = &transmitDescriptorRing[0];
        receivedDescriptor = &receiveDescriptorRing[0];
        EMAC->transmitDMA = reinterpret_cast<Natural32>(transmitableDescriptor);
        EMAC->receiveDMA = reinterpret_cast<Natural32>(receivedDescriptor);
        EMAC->enableReceiver(true);
    }

    void poll() {
        auto EMAC = AllwinnerEMAC::instances[0].address;
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
                    reinterpret_cast<Frame*>(transmitedDescriptor->bufferAddress));
                while(transmitedDescriptor != transmitPeekDescriptor) {
                    transmitedDescriptor->status.raw = 0;
                    transmitedDescriptor->control.bufferSize = 0;
                    transmitedDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitedDescriptor->next);
                }
                transmitedDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitedDescriptor->next);
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
                    reinterpret_cast<Frame*>(receivedDescriptor->bufferAddress));
                while(receivedDescriptor != receivePeekDescriptor) {
                    receivedDescriptor->status.DMAOwnership = 1;
                    receivedDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivedDescriptor->next);
                }
                receivedDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivedDescriptor->next);
                length = 0;
            }
            receivePeekDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivePeekDescriptor->next);
        }
        if(EMAC->receiveDMAStatus.status == AllwinnerEMAC::ReceiveStopped ||
           EMAC->receiveDMAStatus.status == AllwinnerEMAC::ReceiveSuspended) {
            EMAC->receiveDMA = reinterpret_cast<Natural32>(receivedDescriptor);
            EMAC->enableReceiver(true);
        }
    }

    Frame* prepareTransmit(Natural16 totalLength) {
        auto descriptor = transmitableDescriptor;
        auto remainingLength = totalLength;
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
        return reinterpret_cast<Frame*>(descriptor->bufferAddress);
    }

    void transmit(Frame* frame) {
        frame->correctEndian();

        auto UART = AllwinnerUART::instances[0].address;
        for(Natural16 j = 0; j < 128; ++j)
            UART->putHex(reinterpret_cast<Natural8*>(frame)[j]);
        puts(" payload");

        auto index = (reinterpret_cast<Natural32>(frame)-2-reinterpret_cast<Natural32>(transmitBuffers))/bufferSize;
        auto descriptor = &transmitDescriptorRing[index];
        descriptor->status.DMAOwnership = 1;
        auto EMAC = AllwinnerEMAC::instances[0].address;
        if(EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitStopped ||
           EMAC->transmitDMAStatus.status == AllwinnerEMAC::TransmitSuspended) {
            transmitedDescriptor = descriptor;
            EMAC->transmitDMA = reinterpret_cast<Natural32>(descriptor);
            EMAC->enableTransmitter(true);
        }
    }
};
