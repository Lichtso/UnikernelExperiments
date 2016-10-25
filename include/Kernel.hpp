#include <Hardware/AllwinnerA64.hpp>

void main();

extern "C" {
    __attribute__((section(".entry")))
    void _start() {
        main();
    }
}
