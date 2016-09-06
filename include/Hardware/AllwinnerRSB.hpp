#include "AllwinnerUART.hpp"

struct AllwinnerRSB {
    static const struct Instance {
        volatile AllwinnerRSB* address;
        Natural8 interruptPort;
    } instances[];

    union {
        struct {
            Natural32 softReset : 1,
                      globalInterruptEnable : 1,
                      pad0 : 4,
                      abortTransmission : 1,
                      startTransmission : 1;
        };
        Natural32 raw;
    } control;
    union {
        struct {
            Natural32 divisor : 8,
                      delay : 3;
        };
        Natural32 raw;
    } clockControl;
    union {
        struct {
            Natural32 transferComplete : 1,
                      transferError : 1,
                      loadingBusy : 1,
                      pad0 : 5,
                      errorData : 4,
                      pad1 : 4,
                      errorAcknowledgement : 1;
        };
        Natural32 raw;
    } interruptEnable,
      status;
    union {
        Natural8 byte;
        Natural32 raw;
    } dataAddress;
    Natural32 pad0;
    union {
        struct {
            Natural32 bytesMinusOne : 3,
                      pad0 : 1,
                      read : 1;
        };
        Natural32 raw;
    } dataLength;
    Natural32 dataBuffer, pad1;
    union {
        struct {
            Natural32 cdEnable : 1,
                      cdBit : 1,
                      ckEnable : 1,
                      ckBit : 1,
                      cdState : 1,
                      ckState : 1;
        };
        Natural32 raw;
    } lineControl;
    // PMU : performance monitoring unit
    union {
        struct {
            Natural32 deviceAddress : 8,
                      modeControlRegisterAddress : 8,
                      initialData : 8,
                      pad0 : 7,
                      start : 1;
        };
        Natural32 raw;
    } pmuModeControl;
    union {
        Natural8 byte;
        Natural32 raw;
    } command;
    union {
        struct {
            Natural32 hardwareAddress : 16,
                      runTimeAddress : 8;
        };
        Natural32 raw;
    } deviceAddress;

    bool initialize() volatile {
        clockControl.divisor = 3;
        clockControl.delay = 1;
        control.softReset = 1;
        while(control.softReset);

        pmuModeControl.raw = 0x807C3E00;
        while(pmuModeControl.start);
        if(status.transferError || !status.transferComplete) {
            puts("[FAIL] RSB DMC");
            return false;
        } else {
            puts("[ OK ] RSB DMC");
            return true;
        }
    }
    bool transfer(Natural8 cmd) volatile {
        command.byte = cmd;
        control.startTransmission = 1;
        while(control.startTransmission);
        if(status.transferError || !status.transferComplete) {
            puts("[FAIL] RSB transfer");
            return false;
        } else {
            puts("[ OK ] RSB transfer");
            return true;
        }
    }
    bool setRunTimeAddress() volatile {
        return transfer(0xE8);
    }
    bool transfer(Natural8 cmd, Natural8 address) volatile {
        dataAddress.byte = address;
        return transfer(cmd);
    }
    bool read(Natural8 address, Natural8& data) volatile {
        bool success = transfer(0x8B, address);
        data = dataBuffer;
        return success;
    }
    bool read(Natural8 address, Natural16& data) volatile {
        bool success = transfer(0x9C, address);
        data = dataBuffer;
        return success;
    }
    bool read(Natural8 address, Natural32& data) volatile {
        bool success = transfer(0xA6, address);
        data = dataBuffer;
        return success;
    }
    bool write(Natural8 address, Natural8 data) volatile {
        dataBuffer = data;
        return transfer(0x4E, address);
    }
    bool write(Natural8 address, Natural16 data) volatile {
        dataBuffer = data;
        return transfer(0x59, address);
    }
    bool write(Natural8 address, Natural32 data) volatile {
        dataBuffer = data;
        return transfer(0x63, address);
    }
};
