#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <Arduino.h>

namespace ClockManager
{
    // Connects to the NTP server (non-blocking beyond a bounded startup
    // wait) and establishes the initial epoch/millis baseline.
    void begin();

    // Call every loop() iteration. Checks (without blocking) whether it
    // is time for a periodic resync and performs it if so.
    void update();

    // Current wall-clock time as "HH:MM:SS" (24-hour format).
    String getTimeString();

    // Current Unix epoch time, extrapolated from the last sync using millis().
    unsigned long getEpochTime();

    // True once at least one successful NTP sync has occurred.
    bool isSynced();
}

#endif // CLOCK_MANAGER_H
