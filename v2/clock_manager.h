#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "config.h"

// Owns the NTP client and exposes the current wall-clock time.
// Time is kept up to date internally between syncs (the NTPClient
// library extrapolates using millis() internally), and we force an
// explicit resync on a fixed schedule as required.
class ClockManager {
public:
    ClockManager();

    // Starts UDP + NTP client and performs the first blocking sync
    // (only done once, at boot, so the clock is correct before use).
    void begin();

    // Non-blocking. Call every loop() iteration.
    // Handles periodic internal updates and scheduled resyncs.
    void update();

    // Returns "HH:MM:SS" in 24-hour format.
    // (Not const: the underlying NTPClient getters are non-const.)
    String getFormattedTime();

    // True once the first successful sync has completed.
    bool isSynced() const { return synced; }

private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;

    bool synced;
    unsigned long lastResyncMillis;

    static String pad2(int value);
};

#endif // CLOCK_MANAGER_H
