#include <UDP.hpp>

void MAC::transmited(Natural32 errors, Natural32 length, MAC::Frame* frame) {
    if(errors)
        puts("Transmit errors");
    else
        puts("Transmit success");
}

void MAC::received(Natural32 errors, Natural32 length, MAC::Frame* frame) {
    frame->correctEndian();

    auto UART = AllwinnerUART::instances[0].address;
    if(errors) {
        UART->putHex(errors);
        puts(" receive error");
        return;
    }
    UART->putHex(errors);
    puts(" receive success");

    for(Natural16 j = 0; j < 6; ++j)
        UART->putHex(frame->destinationAddress[j]);
    puts(" destination MAC");
    for(Natural16 j = 0; j < 6; ++j)
        UART->putHex(frame->sourceAddress[j]);
    puts(" source MAC");

    switch(frame->type) {
        case IPv4::protocolID:
            IPv4::received(reinterpret_cast<IPv4::Packet*>(frame->payload));
            break;
        case IPv6::protocolID:
            IPv6::received(reinterpret_cast<IPv6::Packet*>(frame->payload));
            break;
        default:
            UART->putHex(frame->type);
            puts(" unknown protocol");
            break;
    }
    puts("");
}

void IPv4::received(IPv4::Packet* packet) {
    puts("IPv4");

    auto UART = AllwinnerUART::instances[0].address;
    if(packet->headerChecksum() != 0) {
        puts("Checksum error");
        return;
    }
    packet->correctEndian();

    for(Natural16 j = 0; j < 4; ++j)
        UART->putHex(packet->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 4; ++j)
        UART->putHex(packet->sourceAddress.bytes[j]);
    puts(" source IP");

    UART->putHex(packet->getPayloadLength());
    puts(" payloadLength");
    for(Natural16 j = 0; j < packet->getPayloadLength(); ++j)
        UART->putHex(packet->getPayload()[j]);
    puts(" payload");

    switch(packet->protocol) {
        case ICMPv4::protocolID:
            handle<ICMPv4>(packet);
            break;
        case TCP::protocolID:
            handle<TCP>(packet);
            break;
        case UDP::protocolID:
            handle<UDP>(packet);
            break;
        default:
            UART->putHex(packet->protocol);
            puts(" unknown protocol");
            break;
    }
}

void IPv6::received(IPv6::Packet* packet) {
    packet->correctEndian();
    puts("IPv6");

    auto UART = AllwinnerUART::instances[0].address;
    for(Natural16 j = 0; j < 16; ++j)
        UART->putHex(packet->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 16; ++j)
        UART->putHex(packet->sourceAddress.bytes[j]);
    puts(" source IP");

    UART->putHex(packet->payloadLength);
    puts(" payloadLength");
    for(Natural16 j = 0; j < packet->payloadLength; ++j)
        UART->putHex(packet->payload[j]);
    puts(" payload");

    switch(packet->nextHeader) {
        case ICMPv6::protocolID:
            handle<ICMPv6>(packet);
            break;
        case TCP::protocolID:
            handle<TCP>(packet);
            break;
        case UDP::protocolID:
            handle<UDP>(packet);
            break;
        default:
            UART->putHex(packet->nextHeader);
            puts(" unknown protocol");
            break;
    }
}
