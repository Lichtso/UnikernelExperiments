typedef char unsigned Natural8;
typedef char Integer8;
typedef short unsigned Natural16;
typedef short Integer16;
typedef unsigned Natural32;
typedef int Integer32;
typedef float Float32;
typedef long long unsigned Natural64;
typedef long long int Integer64;
typedef double Float64;

struct A64_PortController {
    static const struct Instance {
        volatile A64_PortController* address;
    } instances[];

    struct {
        Natural32
            configure[4],
            data,
            multiDriving[2],
            pull[2];
    } banks[8];
    Natural8 pad0[224];
    struct {
        Natural32
            PIOInterrruptConfigure[4],
            PIOInterruptControl,
            PIOInterruptStatus,
            PIOInterruptDebounce,
            pad0;
    } interruptBanks[3];
};

const struct A64_PortController::Instance A64_PortController::instances[] = {
    { reinterpret_cast<A64_PortController*>(0x01C20800) }
};
