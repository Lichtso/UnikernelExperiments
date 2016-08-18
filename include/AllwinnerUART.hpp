#include <AllwinnerTimer.hpp>

struct AllwinnerUART {
    static const struct Instance {
        volatile AllwinnerUART* address;
        Natural8 interruptPort, DMAPort;
    } instances[];

    enum InterruptID {
        None = 1,
        ReceiverLineStatus = 6,
        ReceivedDataAvailable = 4,
        CharacterTimeoutIndication = 12,
        TransmitHoldingRegisterEmpty = 2,
        ModemStatus = 0,
        BusyDetectIndication = 7
    };

    union {
        Natural8 receiverBuffer, transmitHolding,
                 divisorLatchLow;
        Natural32 pad0;
    };
    union {
        struct {
            Natural32 receivedDataAvailable : 1,
                      transmitHoldingRegisterEmpty : 1,
                      receiverLineStatus : 1,
                      modemStatus : 1,
                      pad0 : 3,
                      programmableTHREInterruptMode : 1;
        } interruptEnable;
        Natural8 divisorLatchHigh;
    };
    union {
        const struct {
            InterruptID interruptID : 4;
            Natural32 pad0 : 2,
                      FIFOsEnabled : 2;
        } interruptIdentity;
        struct {
            Natural32 enable : 1,
                      receiveReset : 1,
                      transmitReset : 1,
                      DMAMode : 1,
                      transmitEmptyTrigger : 2,
                      receiveTrigger : 2;
        } FIFOControl;
    };
    union {
        struct {
            Natural32 dataLengthSelect : 2,
                      numberOfStopBits : 1,
                      parityEnable : 1,
                      evenParitySelect : 2,
                      breakControlBit : 1,
                      divisorLatchAccessBit : 1;
        };
        Natural32 raw;
    } lineControl;
    union {
        struct {
            Natural32 dataTerminalReady : 1,
                      requestToSend : 1,
                      pad0 : 2,
                      loopBackMode : 1,
                      autoFlowControlEnable : 1,
                      IrDASIRModeEnable : 1;
        };
        Natural32 raw;
    } modemControl;
    const union {
        struct {
            Natural32 dataReady : 1,
                      overrunError : 1,
                      parityError : 1,
                      framingError : 1,
                      breakInterrupt : 1,
                      transmitHoldingRegisterEmpty : 1,
                      transmitterEmpty : 1,
                      receiveDataErrorInFIFO : 1;
        };
        Natural32 raw;
    } lineStatus;
    const union {
        struct {
            Natural32 deltaClearToSend : 1,
                      deltaDataSetReady : 1,
                      trailingEdgeRingIndicator : 1,
                      deltaDataCarrierDetect : 1,
                      lineStateOfClearToSend : 1,
                      lineStateOfDataSetReady : 1,
                      lineStateOfRingIndicator : 1,
                      lineStateOfDataCarrierDetect : 1;
        };
        Natural32 raw;
    } modemStatus;
    const Natural8 pad1[96];
    const union {
        struct {
            Natural32 busy : 1,
                      transmitFIFONotFull : 1,
                      transmitFIFOEmpty : 1,
                      receiveFIFONotEmpty : 1,
                      receiveFIFOFull : 1;
        };
        Natural32 raw;
    } status;
    const union {
        struct {
            Natural32 level : 6;
        };
        Natural32 raw;
    } transmitFIFOLevel, receiveFIFOLevel;
    const Natural8 pad2[28];
    union {
        struct {
            Natural32 haltTransmit : 1,
                      changeWhileBusyEnable : 1,
                      changeWhileBusyUpdate : 1,
                      pad0 : 1,
                      SIRTransmitPulsePolarityInvert : 1,
                      SIRReceivePulsePolarityInvert : 1;
        };
        Natural32 raw;
    } halt;

    void initialize() volatile {
        halt.haltTransmit = 1;
        lineControl.divisorLatchAccessBit = 1;
        divisorLatchHigh = 0;
        divisorLatchLow = 13; // 24000000 Hz / (16 * 115200 baud)
        lineControl.divisorLatchAccessBit = 0;
        lineControl.dataLengthSelect = 3;
        // FIFOControl.enable = 1;
        // FIFOControl.receiveReset = 1;
        // FIFOControl.transmitReset = 1;
        halt.haltTransmit = 0;

        ::puts("[ OK ] UART0");
    }
    Natural8 getc() volatile {
        while(!lineStatus.dataReady);
        return receiverBuffer;
    }
    void putc(Natural8 value) volatile {
        while(!lineStatus.transmitHoldingRegisterEmpty);
        transmitHolding = value;
    }
    template<typename Type>
    void putHex(Type value) volatile {
        for(Natural8 i = sizeof(value)*8; i > 0; i -= 4) {
            Natural8 nibble = (value>>(i-4))&0xF;
            putc((nibble < 0xA) ? nibble+'0' : nibble-0xA+'A');
        }
    }
    void puts(const char* str) volatile {
        while(*str)
            putc(*str++);
    }
};
