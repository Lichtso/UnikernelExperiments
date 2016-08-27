#include "UDP.hpp"

void MAC::transmited(Natural32 errors, Natural32 length, MAC::Frame* frame) {
    auto UART = AllwinnerUART::instances[0].address;
    if(errors) {
        UART->putHex(errors);
        puts(" transmit errors");
        return;
    }
    UART->putHex(length);
    puts(" transmit success");
}

void MAC::received(Natural32 errors, Natural32 length, MAC::Frame* frame) {
    auto UART = AllwinnerUART::instances[0].address;
    for(Natural16 j = 0; j < 128; ++j)
        UART->putHex(reinterpret_cast<Natural8*>(frame)[j]);
    puts(" frame");

    frame->correctEndian();

    if(errors) {
        UART->putHex(errors);
        puts(" receive error");
        return;
    }
    UART->putHex(length);
    puts(" receive success");

    for(Natural16 j = 0; j < 6; ++j)
        UART->putHex(frame->destinationAddress.bytes[j]);
    puts(" destination MAC");
    for(Natural16 j = 0; j < 6; ++j)
        UART->putHex(frame->sourceAddress.bytes[j]);
    puts(" source MAC");

    switch(frame->type) {
        case IPv4::protocolID:
            IPv4::received(frame, reinterpret_cast<IPv4::Packet*>(frame->payload));
            break;
        case IPv6::protocolID:
            IPv6::received(frame, reinterpret_cast<IPv6::Packet*>(frame->payload));
            break;
        default:
            UART->putHex(frame->type);
            puts(" unknown protocol");
            break;
    }
    puts("");
}

void IPv4::received(MAC::Frame* macFrame, IPv4::Packet* packet) {
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

    switch(packet->protocol) {
        case ICMPv4::protocolID:
            redirectToDriver<ICMPv4>(macFrame, packet);
            break;
        case TCP::protocolID:
            redirectToDriver<TCP>(macFrame, packet);
            break;
        case UDP::protocolID:
            redirectToDriver<UDP>(macFrame, packet);
            break;
        default:
            UART->putHex(packet->protocol);
            puts(" unknown protocol");
            break;
    }
}

void IPv6::received(MAC::Frame* macFrame, IPv6::Packet* packet) {
    packet->correctEndian();
    puts("IPv6");

    auto UART = AllwinnerUART::instances[0].address;
    for(Natural16 j = 0; j < 16; ++j)
        UART->putHex(packet->destinationAddress.bytes[j]);
    puts(" destination IP");
    for(Natural16 j = 0; j < 16; ++j)
        UART->putHex(packet->sourceAddress.bytes[j]);
    puts(" source IP");

    switch(packet->nextHeader) {
        case ICMPv6::protocolID:
            redirectToDriver<ICMPv6>(macFrame, packet);
            break;
        case TCP::protocolID:
            redirectToDriver<TCP>(macFrame, packet);
            break;
        case UDP::protocolID:
            redirectToDriver<UDP>(macFrame, packet);
            break;
        default:
            UART->putHex(packet->nextHeader);
            puts(" unknown protocol");
            break;
    }
}
