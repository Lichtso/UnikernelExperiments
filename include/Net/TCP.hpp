#include "Icmp.hpp"

#define setEstablished() { \
    localInitialSequenceNumber = ++localSequenceNumber; \
    remoteInitialSequenceNumber = receivedTcpPacket->sequenceNumber; \
    status = Established; \
}

#define setTimedClose() { \
    Natural64 now = Clock::getUptimeScaledBy(timerPrecisionScale); \
    retryTimer = now+timerPrecisionScale; /* TODO: Timing */ \
    status = TimedClose; \
}

struct Tcp {
    static constexpr Natural8 protocolID = 6;
    static const Natural64 timerPrecisionScale = 1000; // 1 kHz = 1 ms

    enum OptionType {
        End = 0,
        Padding = 1,
        MaximumSegmentSize = 2,
        WindowScale = 3,
        SelectiveAcknowledgementPermitted = 4,
        SelectiveAcknowledgement = 5,
        Timestamp = 8
    };

    struct Packet {
        Natural16 sourcePort,
                  destinationPort;
        Natural32 sequenceNumber,
                  acknowledgmentNumber;
        Natural16 ns : 1,
                  pad0 : 3,
                  dataOffset : 4,
                  finish : 1,
                  synchronize : 1,
                  reset : 1,
                  push : 1,
                  acknowledgment : 1,
                  urgent : 1,
                  ece : 1,
                  congestionWindowReduced : 1;
        Natural16 windowSize,
                  checksum,
                  urgentPointer;
        Natural8 options[0];

        Natural8* getPayload() {
            return reinterpret_cast<Natural8*>(this)+dataOffset*4;
        }

        void correctEndian() {
            swapEndian(sourcePort);
            swapEndian(destinationPort);
            swapEndian(sequenceNumber);
            swapEndian(acknowledgmentNumber);
            swapEndian(windowSize);
            swapEndian(checksum);
            swapEndian(urgentPointer);
        }

        void prepareTransmit(Ipv4::Packet* ipPacket, Natural16 optionsLength, Natural16 payloadLength) {
            ipPacket->protocol = protocolID;
            ipPacket->totalLength = sizeof(Packet)+optionsLength+payloadLength;
            dataOffset = (sizeof(Packet)+optionsLength+3)/4;
            correctEndian();
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Tcp>();
        }

        void prepareTransmit(Ipv6::Packet* ipPacket, Natural16 optionsLength, Natural16 payloadLength) {
            ipPacket->nextHeader = protocolID;
            ipPacket->payloadLength = sizeof(Packet)+optionsLength+payloadLength;
            dataOffset = (sizeof(Packet)+optionsLength+3)/4;
            correctEndian();
            checksum = 0;
            checksum = ipPacket->payloadChecksum<Tcp>();
        }
    };

    struct Connection {
        // Transmission Timestamps
        // Congestion control, Slow Start, Congestion Avoidance, TCP-Reno
        Mac::Interface* macInterface;
        Natural8 *receiveBuffer, *transmitBuffer; // TODO: Ring buffer
        Natural64 localSequenceNumber = 0, localAcknowledgment = 0, remoteAcknowledgment = 0, retryTimer = 0; // TODO: Handle sequence number overflow
        IpAddress localAddress, remoteAddress;
        Natural32 localMaximumSegmentSize = 1440, remoteMaximumSegmentSize = 1440,
                  localWindowSize = 1024*32, remoteWindowSize = 0,
                  localInitialSequenceNumber = 0, remoteInitialSequenceNumber = 0;
        Natural16 localPort = 0, remotePort = 0;
        Natural8 localWindowScale = 5, remoteWindowScale = 0, version = 6, retryCounter = 0;
        bool selectiveAcknowledgmentEnabled = false, remoteFinished = false;
        enum Status {
            Closed,
            Listen,
            SynSent,
            SynReceived,
            Established,
            FinSent,
            WaitingForRemoteToFinish,
            TimedClose,
            Finished
        } status = Closed;

        struct SelectiveAcknowledgmentBlock {
            Natural32 begin, end;
        };
        // TODO: SACK Ring buffers

        bool transmit() {
            Natural8 optionsLength = (status == SynSent || status == SynReceived) ? 12 : 0;
            auto macFrame = macInterface->createFrame(sizeof(Ipv6::Packet)+sizeof(Packet)+optionsLength);
            if(!macFrame)
                return false;
            Packet* tcpPacket;
            Ipv4::Packet* ipv4Packet = reinterpret_cast<Ipv4::Packet*>(macFrame->payload);
            Ipv6::Packet* ipv6Packet = reinterpret_cast<Ipv6::Packet*>(macFrame->payload);
            switch(version) {
                case 4: {
                    ipv4Packet->destinationAddress = remoteAddress.v4;
                    ipv4Packet->sourceAddress = localAddress.v4;
                    tcpPacket = reinterpret_cast<Packet*>(ipv4Packet->getPayload()); // TODO: Set internetHeaderLength first
                } break;
                case 6: {
                    ipv6Packet->destinationAddress = remoteAddress.v6;
                    ipv6Packet->sourceAddress = localAddress.v6;
                    tcpPacket = reinterpret_cast<Packet*>(ipv6Packet->payload);
                } break;
                default:
                    return false;
            }
            memset(tcpPacket, 0, sizeof(Packet));
            tcpPacket->destinationPort = remotePort;
            tcpPacket->sourcePort = localPort;
            tcpPacket->windowSize = (optionsLength) ? min(65535U, localWindowSize) : localWindowSize>>localWindowScale;
            tcpPacket->sequenceNumber = localSequenceNumber;
            if(status != SynSent) {
                tcpPacket->acknowledgmentNumber = localAcknowledgment;
                tcpPacket->acknowledgment = 1;

                auto uart = AllwinnerUART::instances[0].address;
                uart->putDec(localAcknowledgment-remoteInitialSequenceNumber+1);
                puts(" localAcknowledgment");

                /*if(selectiveAcknowledgmentEnabled) {
                    tcpPacket->options[0] = SelectiveAcknowledgement;
                    tcpPacket->options[1] = 0;
                    struct SelectiveAcknowledgmentBlock blocks[4];
                    // TODO
                    tcpPacket->options[1] *= 8;
                    memcpy(&tcpPacket->options[2], blocks, tcpPacket->options[1]);
                    tcpPacket->options[1] += 2;
                }*/
            }
            if(optionsLength) {
                tcpPacket->synchronize = 1;
                tcpPacket->options[0] = MaximumSegmentSize;
                tcpPacket->options[1] = 4;
                tcpPacket->options[2] = localMaximumSegmentSize>>8;
                tcpPacket->options[3] = localMaximumSegmentSize&0xFF;
                tcpPacket->options[4] = WindowScale;
                tcpPacket->options[5] = 3;
                tcpPacket->options[6] = localWindowScale;
                tcpPacket->options[7] = SelectiveAcknowledgementPermitted;
                tcpPacket->options[8] = 2;
                tcpPacket->options[9] = End;
                tcpPacket->options[10] = End;
                tcpPacket->options[11] = End;
            } else if(remoteAcknowledgment < localSequenceNumber) {
                // TODO: Transmit data here
            } else if(status == FinSent)
                tcpPacket->finish = 1;
            switch(version) {
                case 4:
                    tcpPacket->prepareTransmit(ipv4Packet, optionsLength, 0);
                    ipv4Packet->prepareTransmit(macFrame);
                    break;
                case 6:
                    tcpPacket->prepareTransmit(ipv6Packet, optionsLength, 0);
                    ipv6Packet->prepareTransmit(macFrame);
                    break;
            }
            macInterface->transmit(macFrame);
            Natural64 now = Clock::getUptimeScaledBy(timerPrecisionScale);
            retryTimer = now+timerPrecisionScale; // TODO: Timing
            return true;
        }

        bool received(Mac::Interface* receivingMacInterface, IpPacket* receivedIpPacket) {
            if(status == Closed)
                return false;

            Packet* receivedTcpPacket;
            Natural32 payloadLength;
            switch(version) {
                case 4:
                    receivedTcpPacket = reinterpret_cast<Packet*>(receivedIpPacket->v4.getPayload());
                    if(status != Listen && (remoteAddress.v4 != receivedIpPacket->v4.sourceAddress || remotePort != receivedTcpPacket->sourcePort))
                        return true;
                    payloadLength = receivedIpPacket->v4.getPayloadLength();
                    localAddress.v4 = receivedIpPacket->v4.destinationAddress;
                    break;
                case 6:
                    receivedTcpPacket = reinterpret_cast<Packet*>(receivedIpPacket->v6.payload);
                    if(status != Listen && (remoteAddress.v6 != receivedIpPacket->v6.sourceAddress || remotePort != receivedTcpPacket->sourcePort))
                        return true;
                    payloadLength = receivedIpPacket->v6.payloadLength;
                    localAddress.v6 = receivedIpPacket->v6.destinationAddress;
                    break;
            }
            payloadLength -= receivedTcpPacket->dataOffset*4;
            macInterface = receivingMacInterface;

            remoteWindowSize = receivedTcpPacket->windowSize<<remoteWindowScale;
            Natural8 *pos = receivedTcpPacket->options,
                     *optionsEnd = reinterpret_cast<Natural8*>(receivedTcpPacket)+receivedTcpPacket->dataOffset*4;
            while(pos < optionsEnd) {
                switch(*(pos++)) {
                    case End:
                        optionsEnd = pos;
                        break;
                    case Padding:
                        break;
                    case MaximumSegmentSize:
                        ++pos;
                        remoteMaximumSegmentSize = *(pos++)<<8;
                        remoteMaximumSegmentSize |= *(pos++);
                        break;
                    case WindowScale:
                        ++pos;
                        remoteWindowScale = *(pos++);
                        break;
                    case SelectiveAcknowledgementPermitted:
                        ++pos;
                        selectiveAcknowledgmentEnabled = true;
                        break;
                    case SelectiveAcknowledgement: {
                        Natural8 count = *(pos++)-2;
                        struct SelectiveAcknowledgmentBlock blocks[4];
                        memcpy(blocks, pos, count);
                        pos += count;
                        count /= sizeof(SelectiveAcknowledgmentBlock);
                        for(Natural8 i = 0; i < count; ++i) {
                            swapEndian(blocks[i].begin);
                            swapEndian(blocks[i].end);
                            // TODO: remoteSelectiveAcknowledgment
                        }
                    } break;
                    case Timestamp: {
                        ++pos;
                        Natural32 sender, echo;
                        memcpy(&sender, pos, 8);
                        swapEndian(sender);
                        swapEndian(echo);
                        // TODO
                        pos += 8;
                    } break;
                }
            }

            if(receivedTcpPacket->reset) {
                if(status == Listen || status == TimedClose)
                    return true; // ERROR
                status = Closed;
                return true;
            } else if(receivedTcpPacket->synchronize) {
                if(status == Listen) {
                    if(receivedTcpPacket->acknowledgment)
                        return true; // ERROR
                    remoteAddress.v6 = receivedIpPacket->v6.sourceAddress;
                    remotePort = receivedTcpPacket->sourcePort;
                    localSequenceNumber = 0; // TODO: generate sequence number
                    status = SynReceived;
                } else if(status == SynSent) {
                    if(!receivedTcpPacket->acknowledgment)
                        status = SynReceived;
                    else if(receivedTcpPacket->acknowledgmentNumber == localSequenceNumber+1)
                        setEstablished()
                    else
                        return true; // ERROR
                } else if(status != SynReceived)
                    return true; // ERROR
                localAcknowledgment = receivedTcpPacket->sequenceNumber+1;
                transmit();
                return true;
            } else if(status == Listen || status == SynSent)
                return true; // ERROR
            else if(status == SynReceived)
                setEstablished()
            else if(status == FinSent && receivedTcpPacket->acknowledgmentNumber == localSequenceNumber+1) {
                ++localSequenceNumber;
                if(remoteFinished)
                    setTimedClose()
                else
                    status = WaitingForRemoteToFinish;
            }

            if(receivedTcpPacket->acknowledgmentNumber > remoteAcknowledgment)
                remoteAcknowledgment = receivedTcpPacket->acknowledgmentNumber;

            if(receivedTcpPacket->finish) {
                if(receivedTcpPacket->sequenceNumber != localAcknowledgment)
                    return true;
                remoteFinished = true;
                if(status == WaitingForRemoteToFinish)
                    setTimedClose()
            }

            auto remoteSequenceNumber = receivedTcpPacket->sequenceNumber+payloadLength;
            if(receivedTcpPacket->finish)
                ++remoteSequenceNumber;
            if(localAcknowledgment < remoteSequenceNumber) {
                if(receivedTcpPacket->sequenceNumber == localAcknowledgment)
                    localAcknowledgment = remoteSequenceNumber;
                // TODO: localSelectiveAcknowledgment

                if(payloadLength > 0) {
                    Natural8* dst = receiveBuffer+receivedTcpPacket->sequenceNumber-remoteInitialSequenceNumber;
                    memcpy(dst, optionsEnd, payloadLength);
                }
            }

            if(receivedTcpPacket->finish || payloadLength > 0)
                transmit();
            return true;
        }

        bool poll() {
            Natural64 now = Clock::getUptimeScaledBy(timerPrecisionScale);
            if(status == Closed)
                return false;
            if(now < retryTimer ||
               status == Listen ||
               status == WaitingForRemoteToFinish ||
               (status == Established && remoteAcknowledgment == localSequenceNumber))
                return true;
            if(status == TimedClose) {
                status = Finished;
                return true;
            }
            if(++retryCounter > 5) {
                status = Closed;
                return false;
            }
            transmit();
            return true;
        }

        bool listen() {
            if(status != Closed)
                return false;
            status = Listen;
            return true;
        }

        bool connect() {
            if(status != Closed && status != Listen)
                return false;
            localSequenceNumber = 0; // TODO: generate sequence number
            status = SynSent;
            transmit();
            return true;
        }

        bool close() {
            switch(status) {
                case Closed:
                case FinSent:
                case WaitingForRemoteToFinish:
                case TimedClose:
                    return false;
                case Listen:
                case SynSent:
                case Finished:
                    status = Closed;
                    return true;
                case SynReceived:
                case Established:
                    status = FinSent;
                    transmit();
                    return true;
            }
        }
    };

    static bool transmitReset(Mac::Interface* macInterface, Ipv6::Packet* receivedIpPacket) {
        auto receivedTcpPacket = reinterpret_cast<Packet*>(receivedIpPacket->payload);
        auto macFrame = macInterface->createFrame(sizeof(Ipv6::Packet)+sizeof(Packet));
        if(!macFrame)
            return false;
        auto ipPacket = reinterpret_cast<Ipv6::Packet*>(macFrame->payload);
        ipPacket->destinationAddress = receivedIpPacket->sourceAddress;
        ipPacket->sourceAddress = ipPacket->destinationAddress;
        auto tcpPacket = reinterpret_cast<Packet*>(ipPacket->payload);
        memset(tcpPacket, 0, sizeof(Packet));
        tcpPacket->destinationPort = receivedTcpPacket->sourcePort;
        tcpPacket->sourcePort = receivedTcpPacket->destinationPort;
        tcpPacket->prepareTransmit(ipPacket, 0, 0);
        ipPacket->prepareTransmit(macFrame);
        macInterface->transmit(macFrame);
        return true;
    }

    static void poll() {
        connection->poll();
    }

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv4::Packet* ipPacket, Tcp::Packet* tcpPacket) {
        puts("TCP");
        tcpPacket->correctEndian();
    }

    static void received(Mac::Interface* macInterface, Mac::Frame* macFrame, Ipv6::Packet* ipPacket, Tcp::Packet* tcpPacket) {
        puts("TCP");
        tcpPacket->correctEndian();

        // Only one connection, which writes directly to receiveBuffer and closes after the remote finishes
        if(tcpPacket->destinationPort != connection->localPort) {
            transmitReset(macInterface, ipPacket);
            return;
        }
        connection->received(macInterface, reinterpret_cast<IpPacket*>(ipPacket));
        if(connection->remoteFinished)
            connection->close();
    }

    static Connection* connection;
};

Tcp::Connection* Tcp::connection;
