#include "clock_manager.h"
#include "config.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

namespace
{
    WiFiUDP ntpUDP;
    // Timezone offset is applied once here; NTPClient's internal update
    // interval is disabled (0) because resync timing is driven manually
    // from ClockManager::update() instead.
    NTPClient timeClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, 0);

    unsigned long lastSyncCheckMillis = 0;
    bool timeSynced = false;

    // Baseline captured at the moment of the last successful sync.
    // Between syncs, current time is derived purely from millis(),
    // so the clock keeps ticking even if a resync attempt fails.
    unsigned long baseEpoch = 0;
    unsigned long baseMillis = 0;
}

void ClockManager::begin()
{
    timeClient.begin();

    // Bounded, non-blocking-style wait for the first sync: repeatedly
    // attempt an update while yielding to the WiFi/TCP stack, instead
    // of calling delay(). Gives up after NTP_INITIAL_TIMEOUT_MS.
    unsigned long startAttempt = millis();
    bool success = false;

    while (millis() - startAttempt < NTP_INITIAL_TIMEOUT_MS)
    {
        if (timeClient.forceUpdate())
        {
            success = true;
            break;
        }
        yield();
    }

    baseEpoch = success ? timeClient.getEpochTime() : 0;
    baseMillis = millis();
    lastSyncCheckMillis = baseMillis;
    timeSynced = success;
}

void ClockManager::update()
{
    unsigned long now = millis();

    if (now - lastSyncCheckMillis >= NTP_UPDATE_INTERVAL_MS)
    {
        lastSyncCheckMillis = now;

        if (timeClient.forceUpdate())
        {
            baseEpoch = timeClient.getEpochTime();
            baseMillis = millis();
            timeSynced = true;
        }
        // If the sync attempt fails, keep running on the existing
        // baseline and simply try again at the next interval.
    }
}

unsigned long ClockManager::getEpochTime()
{
    unsigned long elapsedSec = (millis() - baseMillis) / 1000UL;
    return baseEpoch + elapsedSec;
}

bool ClockManager::isSynced()
{
    return timeSynced;
}

String ClockManager::getTimeString()
{
    unsigned long epoch = getEpochTime();
    unsigned long secondsOfDay = epoch % 86400UL;

    int hh = secondsOfDay / 3600;
    int mm = (secondsOfDay % 3600) / 60;
    int ss = secondsOfDay % 60;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hh, mm, ss);
    return String(buf);
}
