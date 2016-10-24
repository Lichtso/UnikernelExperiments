#include "Hardware/AArch64Cache.hpp"

struct Clock {
    static Natural64 uptimeCycles, uptimeTicks;
    static const Natural64 cycleInSeconds = 60; // TODO: Far higher

    static void initialize() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        hsTimer->load(cycleInSeconds*AllwinnerHSTimer::baseFrequency, false);
    }

    static void update() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        Natural64 hsTicks = cycleInSeconds*AllwinnerHSTimer::baseFrequency-hsTimer->getCurrentValue();
        if(hsTicks < uptimeTicks)
            ++uptimeCycles;
        uptimeTicks = hsTicks;
    }

    static Natural64 getUptimeScaledBy(Natural32 scale) {
        return uptimeCycles*cycleInSeconds*scale+uptimeTicks/(AllwinnerHSTimer::baseFrequency/scale);
    }

    static void printUptime() {
        update();
        auto uart = AllwinnerUART::instances[0].address;
        uart->putDec(getUptimeScaledBy(1000));
        puts(" ms Uptime");
    }
};

Natural64 Clock::uptimeCycles = 0, Clock::uptimeTicks = 0;
