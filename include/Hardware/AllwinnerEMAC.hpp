#include "AllwinnerDRAM.hpp"

struct AllwinnerEMAC {
    static const struct Instance {
        volatile AllwinnerEMAC* address;
        Natural8 interruptPort;
    } instances[];

    enum DMAStatus {
        TransmitStopped = 0,
        TransmitFetchingDescriptor = 1,
        TransmitWaitingForStatus = 2,
        TransmitPassingFrame = 3,
        TransmitSuspended = 6,
        TransmitClosingDescriptor = 7,

        ReceiveStopped = 0,
        ReceiveFetchingDescriptor = 1,
        ReceiveWaitingForFrame = 3,
        ReceiveSuspended = 4,
        ReceiveClosingDescriptor = 5,
        ReceivePassingFrame = 7
    };

    struct TransmitDescriptor {
        static const Natural32 errorMask = 0x00015706; // 0000 0000 0000 0001 0101 0111 0000 0110
        union {
            struct {
                Natural32 deferHalfDuplex : 1,
                          underflowError : 1,
                          deferError : 1,
                          collisionCount : 4,
                          pad0 : 1,
                          collisionError : 1,
                          contentionError : 1,
                          carrierError : 1,
                          pad1 : 1,
                          payloadChecksumError : 1,
                          pad2 : 1,
                          lengthError : 1,
                          pad3 : 1,
                          headerChecksumError : 1,
                          pad4 : 14,
                          DMAOwnership : 1;
            };
            Natural32 raw;
        } status;
        union {
            struct {
                Natural32 bufferSize : 11,
                          pad0 : 13,
                          chainMode : 1,
                          pad1 : 1,
                          CRCControl : 1,
                          checksumControl : 2,
                          first : 1,
                          last : 1,
                          completionInterruptEnable : 1;
            };
            Natural32 raw;
        } control;
        Natural32 bufferAddress,
                  next;
    };

    struct ReceiveDescriptor {
        static const Natural32 errorMask = 0x000048DB; // 0000 0000 0000 0000 0100 1000 1101 1011
        union {
            struct {
                Natural32 payloadChecksumError : 1,
                          CRCError : 1,
                          pad0 : 1,
                          phyError : 1,
                          lengthError : 1,
                          pad1 : 1,
                          collisionHalfDuplex : 1,
                          headerChecksumError : 1,
                          last : 1,
                          first : 1,
                          pad2 : 1,
                          overflowError : 1,
                          pad3 : 1,
                          SAFilterFail : 1,
                          truncatinationOccured : 1,
                          pad4 : 1,
                          length : 14,
                          DAFilterFail : 1,
                          DMAOwnership : 1;
            };
            Natural32 raw;
        } status;
        union {
            struct {
                Natural32 bufferSize : 11,
                          pad0 : 13,
                          chainMode : 1,
                          pad1 : 6,
                          completionInterruptDisable : 1;
            };
            Natural32 raw;
        } control;
        Natural32 bufferAddress,
                  next;
    };

    union {
        struct {
            Natural32 fullDuplex : 1,
                      loopBack : 1,
                      speed : 2;
        };
        Natural32 raw;
    } basicControl0;
    union {
        struct {
            Natural32 softReset : 1,
                      prioritizeRX : 1,
                      pad0 : 22,
                      burstLength : 6;
        };
        Natural32 raw;
    } basicControl1;
    const union {
        struct {
            Natural32 TXCompleted : 1,
                      TXDMAStopped : 1,
                      TXDMAOwnership : 1,
                      TXTimeout : 1,
                      TXUnderflow : 1,
                      TXEarly : 1,
                      pad0 : 2,
                      RXCompleted : 1,
                      RXDMAOwnership : 1,
                      RXDMAStopped : 1,
                      RXTimeout : 1,
                      RXOverflow : 1,
                      RXEarly : 1,
                      pad1 : 2,
                      RGMIIStatusChanged : 1;
        };
        Natural32 raw;
    } interruptStatus,
      interruptEnable;
    union {
        struct {
            Natural32 pad0 : 30,
                      frameLengthControl : 1,
                      enableTransmitter : 1;
        };
        Natural32 raw;
    } transmitControl0;
    union {
        struct {
            Natural32 flushFIFODisable : 1,
                      DMAFIFOThresholdDisable : 1,
                      pad0 : 6,
                      DMAFIFOThresholdValue : 3,
                      pad1 : 19,
                      DMAEnable : 1,
                      DMAStart : 1;
        };
        Natural32 raw;
    } transmitControl1;
    Natural32 pad0;
    union {
        struct {
            Natural32 enable : 1,
                      zeroQuantaPauseEnable : 1,
                      pad0 : 2,
                      pauseTime : 16,
                      pauseFrameSlot : 2,
                      pad1 : 9,
                      pauseFrameActive : 1;
        };
        Natural32 raw;
    } transmitFlowControl;
    Natural32 transmitDMA;
    union {
        struct {
            Natural32 pad0 : 16,
                      flowControlEnable : 1,
                      unicastPauseFrameEnable : 1,
                      pad1 : 9,
                      checksumEnable : 1,
                      stripFCSOnShortFrames : 1,
                      jumboFrameEnable : 1,
                      truncateFramesDisable : 1,
                      enableReceiver : 1;
        };
        Natural32 raw;
    } receiveControl0;
    union {
        struct {
            Natural32 flushFrameDisable : 1,
                      DMAFIFOThresholdDisable : 1,
                      forwardShortFrames : 1,
                      forwardErroredFrames : 1,
                      DMAFIFOThresholdValue : 2,
                      pad0 : 14,
                      flowControlEnableThreshold : 2,
                      flowControlDisableThreshold : 2,
                      flowControlThresholdEnable : 1,
                      pad1 : 5,
                      DMAEnable : 1,
                      DMAStart : 1;
        };
        Natural32 raw;
    } receiveControl1;
    Natural32 pad1[2], receiveDMA;
    union {
        struct {
            Natural32 allPass : 1,
                      hybridFilterMode: 1,
                      pad0 : 2,
                      invertDA : 1,
                      invertSA : 1,
                      filterSAEnable : 1,
                      pad1 : 1,
                      hashUnicastEnable : 1,
                      hashMulticastEnable : 1,
                      pad2 : 2,
                      filterControlFrames : 2,
                      pad3 : 2,
                      allMulticastPass : 1,
                      noBroadcastPass : 1,
                      pad4 : 13,
                      addressFilterDisable : 1;
        };
        Natural32 raw;
    } receiveFrameFilter;
    Natural32 pad2, receiveHashTableUpper, receiveHashTableLower;
    union {
        struct {
            Natural32 busy : 1,
                      writeMode : 1,
                      pad0 : 2,
                      registerAddress : 5,
                      pad1 : 4,
                      deviceAddress : 5,
                      pad2 : 3,
                      MDCClockDivisor : 3;
        };
        Natural32 raw;
    } MIICommandRegister;
    Natural32 MIIData,
              MAC0High, MAC0Low;
    struct {
        struct {
            Natural32 high : 16,
                      pad0 : 8,
                      byteControlMask: 6,
                      sourceAddressMode : 1,
                      enable : 1;
        };
        Natural32 low;
    } MACx[7];
    Natural32 pad3[8];
    union {
        struct {
            Natural32 status : 3;
        };
        Natural32 raw;
    } transmitDMAStatus;
    const Natural32 transmitDMACurrentDescriptor;
    const Natural32 transmitDMACurrentBuffer;
    Natural32 pad4;
    union {
        struct {
            Natural32 status : 3;
        };
        Natural32 raw;
    } receiveDMAStatus;
    const Natural32 receiveDMACurrentDescriptor;
    const Natural32 receiveDMACurrentBuffer;
    Natural32 pad5;
    union {
        struct {
            Natural32 fullDuplex : 1,
                      linkSpeed : 2,
                      linkUp : 1;
        };
        Natural32 raw;
    } RGMIIStatus;

    void initialize() volatile {
        basicControl1.softReset = 1;
        while(basicControl1.softReset);

        basicControl0.fullDuplex = 1;
        basicControl0.loopBack = 0;
        basicControl0.speed = 0;
        basicControl1.burstLength = 8;
        MIICommandRegister.MDCClockDivisor = 1;

        if(MIIRead(0, 2) != 0x001C || MIIRead(0, 3) != 0xC915) {
            puts("[FAIL] RTL8211E-VB");
            return;
        } else
            puts("[ OK ] RTL8211E-VB");

        transmitControl0.frameLengthControl = 1;
        transmitControl1.flushFIFODisable = 1;
        transmitControl1.DMAFIFOThresholdDisable = 1;
        transmitControl1.DMAFIFOThresholdValue = 0;
        transmitFlowControl.enable = 0;
        transmitFlowControl.zeroQuantaPauseEnable = 0;
        transmitFlowControl.pauseTime = 0;
        transmitFlowControl.pauseFrameSlot = 0;
        transmitFlowControl.pauseFrameActive = 0;

        receiveControl0.flowControlEnable = 1;
        receiveControl0.unicastPauseFrameEnable = 1;
        receiveControl0.checksumEnable = 0;
        receiveControl0.stripFCSOnShortFrames = 0;
        receiveControl0.jumboFrameEnable = 1;
        receiveControl0.truncateFramesDisable = 0;
        receiveControl1.flushFrameDisable = 0;
        receiveControl1.DMAFIFOThresholdDisable = 0;
        receiveControl1.forwardShortFrames = 1;
        receiveControl1.forwardErroredFrames = 0;
        receiveControl1.DMAFIFOThresholdValue = 1;
        receiveControl1.flowControlEnableThreshold = 0;
        receiveControl1.flowControlDisableThreshold = 0;
        receiveControl1.flowControlThresholdEnable = 0;

        receiveFrameFilter.allPass = 0;
        receiveFrameFilter.hybridFilterMode = 0;
        receiveFrameFilter.invertDA = 0;
        receiveFrameFilter.invertSA = 0;
        receiveFrameFilter.filterSAEnable = 0;
        receiveFrameFilter.hashUnicastEnable = 0;
        receiveFrameFilter.hashMulticastEnable = 0;
        receiveFrameFilter.filterControlFrames = 0;
        receiveFrameFilter.allMulticastPass = 1;
        receiveFrameFilter.noBroadcastPass = 0;
        receiveFrameFilter.addressFilterDisable = 0;
    }

    void waitForLink() volatile {
        while(!link());

        auto uart = AllwinnerUART::instances[0].address;
        uart->puts("[ OK ] ");
        const char* speedStrings[] = { "10 Mbps", "100 Mbps", "1 Gbps", "Unknown speed" };
        uart->puts(speedStrings[MIIRead(0, 17)>>14]);
        puts(" Ethernet link");
    }

    Natural16 MIIRead(Natural8 deviceAddress, Natural8 registerAddress) volatile {
        MIICommandRegister.deviceAddress = deviceAddress;
        MIICommandRegister.registerAddress = registerAddress;
        MIICommandRegister.writeMode = 0;
        MIICommandRegister.busy = 1;
        while(MIICommandRegister.busy);
        return MIIData;
    }

    void MIIWrite(Natural8 deviceAddress, Natural8 registerAddress, Natural16 data) volatile {
        MIIData = data;
        MIICommandRegister.deviceAddress = deviceAddress;
        MIICommandRegister.registerAddress = registerAddress;
        MIICommandRegister.writeMode = 1;
        MIICommandRegister.busy = 1;
        while(MIICommandRegister.busy);
    }

    void enableTransmitter(bool enabled) volatile {
        transmitControl0.enableTransmitter = enabled;
        transmitControl1.DMAEnable = enabled;
        transmitControl1.DMAStart = enabled;
    }

    void enableReceiver(bool enabled) volatile {
        receiveControl0.enableReceiver = enabled;
        receiveControl1.DMAEnable = enabled;
        receiveControl1.DMAStart = enabled;
    }

    bool link() volatile {
        return MIIRead(0, 17)&(1<<10);
    }

    void setMACAddress(Natural8 index, const void* buffer) volatile {
        auto dst = reinterpret_cast<volatile Natural8*>(&MAC0High)+index*8;
        auto src = reinterpret_cast<const Natural8*>(buffer);
        dst[4] = src[0];
        dst[5] = src[1];
        dst[6] = src[2];
        dst[7] = src[3];
        dst[0] = src[4];
        dst[1] = src[5];
    }

    void getMACAddress(Natural8 index, void* buffer) volatile {
        auto src = reinterpret_cast<volatile Natural8*>(&MAC0High)+index*8;
        auto dst = reinterpret_cast<Natural8*>(buffer);
        dst[0] = src[4];
        dst[1] = src[5];
        dst[2] = src[6];
        dst[3] = src[7];
        dst[4] = src[0];
        dst[5] = src[1];
    }
};
