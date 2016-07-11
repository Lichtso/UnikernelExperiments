#include "PL011.hpp"

extern "C" {
    void received() {
        PL011::instances[0]->dataRegister.data = PL011::instances[0]->dataRegister.data;
        PL011::instances[0]->print(" received\n");
    }

    void main() {
        PL011::instances[0]->print("Hello, World!\n");
        PL011::instances[0]->interruptMaskRegister.receive = 1;
        PL190::instance->setSlot(PL011::interrupts[0], reinterpret_cast<Natural64>(received));

        asm(
            "ldr x0, =interruptVector\n"
            "msr VBAR_EL1, x0\n"
            "msr DAIFClr, #2\n"
        );

        while(1);
    }
}
