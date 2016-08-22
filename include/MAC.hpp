#include <AllwinnerA64.hpp>

struct MAC {
    static const Natural16 transmitBufferCount = 128, receiveBufferCount = 128, bufferSize = 1536;

    struct Frame {
        Natural8 destinationAddress[6], sourceAddress[6];
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
    union {
        struct Frame frame;
        Natural8 pad0[bufferSize];
    } transmitBuffers[transmitBufferCount];
    union {
        struct Frame frame;
        Natural8 pad0[bufferSize];
    } receiveBuffers[receiveBufferCount];
    struct AllwinnerEMAC::TransmitDescriptor *transmitedDescriptor, *transmitableDescriptor;
    struct AllwinnerEMAC::ReceiveDescriptor *receivedDescriptor;

    void initialize() {
        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->initialize();

        for(Natural8 i = 0; i < transmitBufferCount; ++i) {
            auto descriptor = &transmitDescriptorRing[i];
            descriptor->bufferAddress = reinterpret_cast<Natural32>(&transmitBuffers[i]);
            descriptor->next = reinterpret_cast<Natural32>(&transmitDescriptorRing[(i+1)%transmitBufferCount]);
            descriptor->status.raw = 0;
            descriptor->status.DMAOwnership = 0;
            descriptor->control.raw = 0;
            descriptor->control.bufferSize = 0;
            descriptor->control.chainMode = 1;
            descriptor->control.checksumEnable = 3;
            descriptor->control.completionInterruptEnable = 1;
        }

        for(Natural8 i = 0; i < receiveBufferCount; ++i) {
            auto descriptor = &receiveDescriptorRing[i];
            descriptor->bufferAddress = reinterpret_cast<Natural32>(&receiveBuffers[i]);
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

        while(1) {
            if(transmitedDescriptor->control.bufferSize == 0) { // Vacant Frame
                EMAC->enableTransmitter(false);
                break;
            }
            if(transmitedDescriptor->status.DMAOwnership) // Pending Frame
                break;
            auto descriptor = transmitedDescriptor;
            Natural32 length = 0;
            while(1) {
                length += descriptor->control.bufferSize;
                if(descriptor->control.last)
                    break;
                descriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(descriptor->next);
            }
            transmited(
                transmitedDescriptor->status.raw&AllwinnerEMAC::TransmitDescriptor::errorMask,
                length,
                reinterpret_cast<Frame*>(transmitedDescriptor->bufferAddress));
            transmitedDescriptor->control.bufferSize = 0;
            transmitedDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitedDescriptor->next);
        }

        while(1) { // TODO: Max count
            if(receivedDescriptor->status.DMAOwnership) // Vacant Frame
                break;
            auto descriptor = receivedDescriptor;
            Natural32 length = 0;
            while(1) {
                length += descriptor->control.bufferSize;
                if(descriptor->status.last)
                    break;
                descriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(descriptor->next);
            }
            received(
                receivedDescriptor->status.raw&AllwinnerEMAC::ReceiveDescriptor::errorMask,
                length,
                reinterpret_cast<Frame*>(receivedDescriptor->bufferAddress));
            receivedDescriptor->status.DMAOwnership = 1;
            receivedDescriptor = reinterpret_cast<AllwinnerEMAC::ReceiveDescriptor*>(receivedDescriptor->next);
        }

        // TODO: Restart receiver if stuck
    }

    Frame* prepareTransmit(Natural16 length) {
        auto descriptor = transmitableDescriptor;
        auto totalLength = length;
        while(1) {
            if(descriptor->control.bufferSize > 0)
                return nullptr;
            auto fragmentLength = min(totalLength, bufferSize);
            totalLength -= fragmentLength;
            if(totalLength == 0)
                break;
            descriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(descriptor->next);
        }

        descriptor = transmitableDescriptor;
        transmitableDescriptor->control.first = true;
        while(1) {
            transmitableDescriptor->control.bufferSize = min(length, bufferSize);
            length -= transmitableDescriptor->control.bufferSize;
            transmitableDescriptor->control.last = (length == 0);
            transmitableDescriptor->status.DMAOwnership = !transmitableDescriptor->control.first;
            if(transmitableDescriptor->control.last)
                break;
            transmitableDescriptor = reinterpret_cast<AllwinnerEMAC::TransmitDescriptor*>(transmitableDescriptor->next);
            transmitableDescriptor->control.first = false;
        }

        return reinterpret_cast<Frame*>(descriptor->bufferAddress);
    }

    void transmit(Frame* frame) {
        auto index = (reinterpret_cast<Natural32>(frame)-reinterpret_cast<Natural32>(transmitBuffers))/bufferSize;
        auto descriptor = &transmitDescriptorRing[index];
        descriptor->status.DMAOwnership = 1;

        auto EMAC = AllwinnerEMAC::instances[0].address;
        EMAC->enableTransmitter(true);
    }
};
