#include <Hardware/AllwinnerA64.hpp>

void main();

extern "C" {
    void _start() {
        main();
    }
}
