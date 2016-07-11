struct PL190 {
    static PL190* const instance;
    static const Natural8 slotCount = 16;

    volatile const Natural32 IRQStatusRegister;
    volatile const Natural32 FIQStatusRegister;
    volatile const Natural32 rawInterruptStatusRegister;
    volatile Natural32 interruptSelectRegister;
    volatile Natural32 interruptEnableRegister;
    volatile Natural32 interruptEnableClearRegister;
    volatile Natural32 softwareInterruptRegister;
    volatile Natural32 softwareInterruptClearRegister;
    volatile union {
        struct {
            Natural32 enable : 1;
        };
        Natural32 raw;
    } protectionEnableRegister;
    const Natural8 reserved1[12];
    volatile Natural32 vectorAddressRegister;
    volatile Natural32 defaultVectorAddressRegister;
    const Natural8 reserved2[200];
    volatile Natural32 vectorAddressRegisters[slotCount];
    const Natural8 reserved3[192];
    volatile union {
        struct {
            Natural32 interruptSource : 5,
                      vectorInterruptEnable : 1;
        };
        Natural32 raw;
    } vectorControlRegisters[slotCount];

    void restoreInterruptEnableRegister() {
        for(Natural8 slot = 0; slot < slotCount; ++slot)
            interruptEnableRegister |= vectorControlRegisters[slot].vectorInterruptEnable<<slot;
    }
    Natural8 setSlot(Natural8 interrupt, Natural32 address) {
        for(Natural8 slot = 0; slot < slotCount; ++slot) {
            if(vectorControlRegisters[slot].vectorInterruptEnable)
                continue;
            interruptSelectRegister &= ~(1<<interrupt);
            interruptEnableRegister = 1<<interrupt;
            vectorAddressRegisters[slot] = address;
            vectorControlRegisters[slot].interruptSource = interrupt;
            vectorControlRegisters[slot].vectorInterruptEnable = 1;
            return slot;
        }
        return slotCount;
    }
    void clearSlot(Natural8 slot) {
        Natural8 interrupt = vectorControlRegisters[slot].interruptSource;
        interruptEnableClearRegister = 1<<interrupt;
        vectorControlRegisters[slot].vectorInterruptEnable = 0;
    }
};

PL190* const PL190::instance = reinterpret_cast<PL190*>(0x10140000);
