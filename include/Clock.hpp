#include "Hardware/AllwinnerA64.hpp"

struct Clock {
    static Natural64 uptimeSeconds;
    static Natural32 uptimeTicks, lastHsTicks;

    static void initialize() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        hsTimer->load(AllwinnerHSTimer::baseFrequency, false);
    }

    static void update() {
        auto hsTimer = AllwinnerHSTimer::instances[0].address;
        Natural64 hsTicks = -(hsTimer->getCurrentValue()|(0xFFULL<<56));
        if(hsTicks < lastHsTicks)
            ++uptimeSeconds;
        lastHsTicks = hsTicks;
    }

    static Natural64 getUptimeScaledBy(Natural32 scale) {
        Natural32 subSecondPart = uptimeTicks/(AllwinnerHSTimer::baseFrequency/scale);
        return uptimeSeconds*scale+subSecondPart;
    }

    static void printUptime() {
        // TODO
    }
};

Natural64 Clock::uptimeSeconds;
Natural32 Clock::uptimeTicks, Clock::lastHsTicks;
