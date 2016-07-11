#include "PL190.hpp"

struct PL011 {
    static PL011* const instances[];
    static const Natural8 interrupts[];

    volatile union {
        struct {
            Natural32 data : 8,
                      framingError : 1,
                      parityError : 1,
                      breakError : 1,
                      overrunError : 1;
        };
        Natural32 raw;
    } dataRegister;
    volatile union {
        struct {
            Natural32 framingError : 1,
                      parityError : 1,
                      breakError : 1,
                      overrunError : 1;
        };
        Natural32 raw;
    } receiveStatusErrorClearRegister;
    const Natural8 reserved1[16];
    volatile const union {
        struct {
            Natural32 clearToSend : 1,
                      dataSetReady : 1,
                      dataCarrierDetect : 1,
                      UARTBusy : 1,
                      receiveEmpty : 1,
                      transmitFull : 1,
                      receiveFull : 1,
                      transmitEmpty : 1,
                      ringIndicator : 1;
        };
        Natural32 raw;
    } flagRegister;
    const Natural8 reserved2[4];
    volatile union {
        struct {
            Natural32 value : 8;
        };
        Natural32 raw;
    } IrDALowPowerCounterRegister;
    volatile union {
        struct {
            Natural32 value : 16;
        };
        Natural32 raw;
    } integerBaudRateRegister;
    volatile union {
        struct {
            Natural32 value : 6;
        };
        Natural32 raw;
    } fractionalBaudRateRegister;
    volatile union {
        struct {
            Natural32 sendBreak : 1,
                      parityEnable : 1,
                      evenParitySelect : 1,
                      twoStopBitsSelect : 1,
                      FIFOEnable : 1,
                      wordLength : 2,
                      stickParitySelect : 1;
        };
        Natural32 raw;
    } lineControlRegister;
    volatile union {
        struct {
            Natural32 UARTEnable : 1,
                      SIREnable : 1,
                      IrDASIRLowPowerMode : 1,
                      reserved : 4,
                      loopBackEnable : 1,
                      transmitEnable : 1,
                      receiveEnable : 1,
                      dataTransmitReady : 1,
                      requestToSend : 1,
                      out : 2,
                      RTSHardwareFlowControlEnable : 1,
                      CTSHardwareFlowControlEnable : 1;
        };
        Natural32 raw;
    } controlRegister;
    volatile union {
        struct {
            Natural32 transmit : 3,
                      receive : 3;
        };
        Natural32 raw;
    } interruptFIFOLevelSelectRegister;
    union Interrupts {
        struct {
            Natural32 RIModem : 1,
                      CTSModem : 1,
                      DCDModem : 1,
                      DSRModem : 1,
                      receive : 1,
                      transmit : 1,
                      receiveTimeout : 1,
                      framingError : 1,
                      parityError : 1,
                      breakError : 1,
                      overrunError : 1;
        };
        Natural32 raw;
    };
    volatile union Interrupts interruptMaskRegister;
    volatile const union Interrupts rawInterruptStatusRegister;
    volatile const union Interrupts maskedInterruptStatusRegister;
    volatile union Interrupts interruptClearRegister;
    volatile union {
        struct {
            Natural32 receiveEnable : 1,
                      transmitEnable : 1,
                      onError : 1;
        };
        Natural32 raw;
    } DMAControlRegister;

    void print(const Integer8* str) {
        while(*str)
            dataRegister.data = *(str++);
    }
    void printBinary(Natural64 value) {
        for(Natural8 i = 0; i < sizeof(value)*8; ++i)
            dataRegister.data = ((value>>i)&1) ? '1' : '0';
    }
};

PL011* const PL011::instances[] = {
    reinterpret_cast<PL011*>(0x101f1000),
    reinterpret_cast<PL011*>(0x101f2000),
    reinterpret_cast<PL011*>(0x101f3000)
};
const Natural8 PL011::interrupts[] = { 12, 13, 14 };
