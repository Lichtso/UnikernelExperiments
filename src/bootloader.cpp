#define access(X) *reinterpret_cast<volatile unsigned int*>(X)

void main() {
    access(0x01C20828) = 0x44; // UART0 TX(8), RX(9)
    access(0x01C20840) = 1<<18; // UART0 RX(9) PullUP

    access(0x01C2006C) |= 1<<16; // BusClock_Gating3 UART0
    access(0x01C202D8) |= 1<<16; // Bus_Software_Reset4 UART0

    access(0x01C2800C) = 0x80; // UART0 divisorLatchAccessBit
    access(0x01C28004) = 0; // UART0 divisorLatchHigh
    access(0x01C28000) = 13; // UART0 divisorLatchLow (115200 baud)
    access(0x01C2800C) = 3; // UART0 dataLengthSelect: 8 bit

    for(const char* str = "Hello, World!\r\n"; *str; ++str) {
        while(!(access(0x01C28014)&(1<<6))); // UART0 lineStatus.transmitHoldingRegisterEmpty
        *reinterpret_cast<volatile char*>(0x01C28000) = *str; // UART0 transmitHolding
    }
}
