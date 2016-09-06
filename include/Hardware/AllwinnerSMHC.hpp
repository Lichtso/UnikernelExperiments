#include "AllwinnerEMAC.hpp"

struct AllwinnerSMHC {
    static const struct Instance {
        volatile AllwinnerSMHC* address;
    } instances[];

    struct Descriptor {
        union {
            struct {
                Natural32 pad0 : 1,
                          interruptOnCompletionDisable : 1,
                          last : 1,
                          first : 1,
                          chainMode : 1,
                          pad1 : 25,
                          error : 1,
                          DMAOwnership : 1;
            };
            Natural32 raw;
        } status;
        union {
            struct {
                Natural32 bufferSize : 15;
            };
            Natural32 raw;
        } control;
        Natural32 bufferAddress,
                  next;
    };

    union {
        struct {
            Natural32 softwareReset : 1,
                      FIFOReset : 1,
                      DMAReset : 1,
                      pad0 : 1,
                      globalInterruptEnable : 1,
                      globalDMAEnable : 1,
                      pad1 : 2,
                      deBounceEnable : 1,
                      pad2 : 1,
                      DDRModeSelect : 1,
                      dataLineTimeUnit : 1,
                      commandLineTimeUnit : 1,
                      pad3 : 18,
                      FIFOAccessMode : 1;
        };
        Natural32 raw;
    } control;
    union {
        struct {
            Natural32 cardClockDivisor : 8,
                      pad0 : 8,
                      cardClockEnable : 1,
                      cardClockOutputControl : 1,
                      pad1 : 13,
                      maskData0 : 1;
        };
        Natural32 raw;
    } clockControl;
    union {
        struct {
            Natural32 response : 8,
                      data : 24;
        };
        Natural32 raw;
    } timeout;
    union {
        struct {
            Natural32 cardWidth : 2;
        };
        Natural32 raw;
    } busWidth;
    union {
        struct {
            Natural32 blockSize : 16;
        };
        Natural32 raw;
    } blockSize;
    Natural32 byteCount;
    union {
        struct {
            Natural32 commandIndex : 6,
                      responseReceiveEnable : 1,
                      longResponse : 1,
                      checkResponseCRC : 1,
                      dataTransferEnable : 1,
                      writeMode : 1,
                      transferMode : 1,
                      stopAutomaticEnable : 1,
                      waitForPending : 1,
                      stopAbortCommand : 1,
                      sendInitialization : 1,
                      pad0 : 5,
                      changeClock : 1,
                      pad1 : 2,
                      bootMode : 2,
                      expectBootAcknowledge : 1,
                      bootAbort : 1,
                      voltageSwitch : 1,
                      pad2 : 2,
                      start : 1;
        };
        Natural32 raw;
    } command;
    Natural32 commandArgument, response[4];
    union {
        struct {
            Natural32 pad0 : 1,
                      responseError : 1,
                      commandComplete : 1,
                      dataTransferComplete : 1,
                      dataTransmitRequest : 1,
                      dataReceiveRequest : 1,
                      responseCRCError : 1,
                      dataCRCError : 1,
                      responseTimout : 1,
                      dataTimeout : 1,
                      dataStarvationTimeout : 1,
                      FIFO : 1,
                      commandBusy : 1,
                      dataStartError : 1,
                      autoCommandDone : 1,
                      dataEndBitError : 1,
                      SDIO : 1,
                      pad1 : 13,
                      cardInserted : 1,
                      cardRemoved : 1;
        };
        Natural32 raw;
    } interruptMask,
      maskedInterruptStatus,
      rawInterruptStatus;
    union {
        struct {
            Natural32 FIFOReceiveReachedThreshold : 1,
                      FIFOTransmitReachedThreshold : 1,
                      FIFOEmpty : 1,
                      FIFOFull : 1,
                      commandFSM : 4,
                      cardPresent : 1,
                      cardDataBusy : 1,
                      dataFSMBusy : 1,
                      responseIndex : 6,
                      FIFOLevel : 9,
                      pad0 : 5,
                      DMARequestSignalState : 1;
        };
        Natural32 raw;
    } status;
    union {
        struct {
            Natural32 transmitTriggerLevel : 8,
                      pad0 : 8,
                      receiveTriggerLevel : 8,
                      pad1 : 4,
                      burstSize : 4;
        };
        Natural32 raw;
    } FIFOThreshold;
    union {
        struct {
            Natural32 hostSendMMCIRQResponse : 1,
                      SDIOReadWait : 1,
                      abortReadData : 1;
        };
        Natural32 raw;
    } FIFOFunctionSelect;
    Natural32 transferredCounter[2], pad0;
    union {
        struct {
            Natural32 speedMode : 4;
        };
        Natural32 raw;
    } CRCStatusDetectControl;
    union {
        struct {
            Natural32 argument : 16;
        };
        Natural32 raw;
    } autoCommand12;
    union {
        struct {
            Natural32 pad0 : 4,
                      sampleTimingPhase : 2,
                      pad1 : 25,
                      modeSelect : 1;
        };
        Natural32 raw;
    } SDNewTimingSet;
    Natural32 pad1[6];
    union {
        struct {
            Natural32 reset : 1;
        };
        Natural32 raw;
    } hardware;
    Natural32 pad2;
    union {
        struct {
            Natural32 reset : 1,
                      fixedBurst : 1,
                      pad0 : 5,
                      IDMACEnable : 1,
                      pad1 : 23,
                      start : 1;
        };
        Natural32 raw;
    } DMAControl;
    Natural32 descriptorListBaseAddress;
    union {
        struct {
            Natural32 transmit : 1,
                      receive : 1,
                      fatalBusError : 1,
                      pad0 : 1,
                      DMAOwnership : 1,
                      cardErrorSummary : 1,
                      pad1 : 2,
                      normalInterruptSummary : 1,
                      abnormalInterruptSummary : 1,
                      errorBits : 3;
        };
        Natural32 raw;
    } DMACStatus, DMACInterruptEnable;
    Natural32 pad3[28];
    union {
        struct {
            Natural32 cardReadThresholdEnable : 1,
                      busyClearInterruptGeneration : 1,
                      cardWriteThresholdEnable : 1,
                      pad0 : 13,
                      cardReadThreshold : 12;
        };
        Natural32 raw;
    } cardThresholdControl;
    Natural32 pad4[2];
    union {
        struct {
            Natural32 halfStart : 1,
                      pad0 : 30,
                      HS400Enable : 1;
        };
        Natural32 raw;
    } eMMC45DDRStartBitDetectionControl;
    union {
        struct {
            Natural32 fromDevice : 7;
        };
        Natural32 raw;
    } responseCRC;
    Natural32 CRCInDataFromDevice[8];
    union {
        struct {
            Natural32 fromDeviceInWriteOperation : 3;
        };
        Natural32 raw;
    } CRCStatus;
    Natural32 pad5[2];
    union {
        struct {
            Natural32 pad0 : 16,
                      commandDrivePhase : 1,
                      dataDrivePhase : 1;
        };
        Natural32 raw;
    } d￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼riveDelayControl;
    union {
        struct {
            Natural32 softwareSampleDelay : 6,
                      pad0 : 1,
                      softwareSampleDelayEnable : 1,
                      sampleDelay : 6,
                      sampleDelayCalibrationDone : 1,
                      sampleDelayCalibrationStart : 1;
        };
        Natural32 raw;
    } sampleDelayControl;
    union {
        struct {
            Natural32 softwareDataStrobeDelay : 6,
                      pad0 : 1,
                      softwareDataStrobeDelayEnable : 1,
                      dataStrobeDelay : 6,
                      dataStrobeDelayCalibrationDone : 1,
                      dataStrobeDelayCalibrationStart : 1;
        };
        Natural32 raw;
    } dataStrobeDelayControl;
    Natural32 pad6[45];
    Natural32 readWriteFIFO;

    void initialize() volatile {
        // sampleDelayControl.raw = 0xA0;
        sampleDelayControl.softwareSampleDelay = 32;
        sampleDelayControl.softwareSampleDelayEnable = 1;
        sampleDelayControl.raw = 0;

        sampleDelayControl.sampleDelayCalibrationStart = 1;
        while(!sampleDelayControl.sampleDelayCalibrationDone);

        // TODO: Step6: Calculate the delay time of one delay cell according to the cycle of SMHC’s clock and the result of calibration.
    }
};
