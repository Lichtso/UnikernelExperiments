#include "Hardware/AArch64Cache.hpp"

struct Clock {
    static Natural64 uptimeClockMetaTicks, uptimeClockTicks;
    static const Natural64 cycleInSeconds = 60; // TODO: Far higher

    static void initialize() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        hsTimer->load(cycleInSeconds*AllwinnerHSTimer::baseFrequency, false);
    }

    static void update() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        Natural64 hsTicks = cycleInSeconds*AllwinnerHSTimer::baseFrequency-hsTimer->getCurrentValue();
        if(hsTicks < uptimeClockTicks)
            ++uptimeClockMetaTicks;
        uptimeClockTicks = hsTicks;
    }

    static Natural64 getUptimeScaledBy(Natural32 scale) {
        return uptimeClockMetaTicks*cycleInSeconds*scale+uptimeClockTicks/(AllwinnerHSTimer::baseFrequency/scale);
    }

    static void printUptime() {
        update();
        auto uart = AllwinnerUART::instances[0].address;
        uart->putDec(getUptimeScaledBy(1000));
        puts(" ms Uptime");
    }

    static void setCycleCounterActive(bool active) {
        Natural64 performanceMonitorsControlRegister;
        asm volatile("mrs %x0, PMCR_EL0\n" : "=r"(performanceMonitorsControlRegister));
        performanceMonitorsControlRegister |= (1<<0);
        asm volatile("msr PMCR_EL0, %x0\n" : : "r"(performanceMonitorsControlRegister));
        if(active)
            asm volatile("msr PMCNTENSET_EL0, %x0\ndsb sy\n" : : "r"(1<<31));
        else
            asm volatile("msr PMCNTENCLR_EL0, %x0\ndsb sy\n" : : "r"(1<<31));
    }

    static Natural64 getCycleCounter() {
        Natural64 performanceMonitorsControlRegister, processorCycles;
        asm volatile("mrs %x0, PMCCNTR_EL0\n" : "=r"(processorCycles));
        asm volatile("mrs %x0, PMCR_EL0\n" : "=r"(performanceMonitorsControlRegister));
        performanceMonitorsControlRegister |= (1<<2);
        asm volatile("msr PMCR_EL0, %x0\ndsb sy\n" : : "r"(performanceMonitorsControlRegister));
        return processorCycles;
    }
};

Natural64 Clock::uptimeClockMetaTicks = 0, Clock::uptimeClockTicks = 0;
