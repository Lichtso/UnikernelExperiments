#include "A64_UART.hpp"

void main() {
    auto UART = A64_UART::instances[0].address;
    UART->initalize();
    UART->puts("Hello, World!\n");
    while(1)
        UART->putc(UART->getc()+1);
}
