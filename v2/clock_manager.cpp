#include "clock_manager.h"

ClockManager::ClockManager()
    : timeClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, NTP_UPDATE_INTERVAL_MS),
      synced(false),
      lastResyncMillis(0)
{
}

void ClockManager::begin() {
    timeClient.begin();

    // Initial sync is intentionally blocking (only happens once at
    // boot before the main loop starts) so we never display an
    // incorrect time. update() below is non-blocking thereafter.
    Serial.println(F("[Clock] Performing initial NTP sync..."));
    int attempts = 0;
    while (!timeClient.forceUpdate() && attempts < 10) {
        attempts++;
        delay(500); // Acceptable here: setup()-time only, not in loop().
    }

    if (attempts < 10) {
        synced = true;
        Serial.println(F("[Clock] Initial NTP sync successful."));
    } else {
        Serial.println(F("[Clock] Initial NTP sync failed, will retry in loop()."));
    }

    lastResyncMillis = millis();
}

void ClockManager::update() {
    // update() is non-blocking: NTPClient only sends a UDP packet
    // when its internal update interval has elapsed, otherwise it
    // just returns immediately having extrapolated from millis().
    if (timeClient.update()) {
        synced = true;
    }

    // Explicit forced resync on the required schedule. forceUpdate()
    // still only performs a single quick UDP exchange, so this stays
    // effectively non-blocking in normal operation.
    unsigned long now = millis();
    if (now - lastResyncMillis >= NTP_RESYNC_INTERVAL_MS) {
        lastResyncMillis = now;
        if (timeClient.forceUpdate()) {
            synced = true;
            Serial.println(F("[Clock] Periodic resync successful."));
        } else {
            Serial.println(F("[Clock] Periodic resync failed, will retry next cycle."));
        }
    }
}

String ClockManager::pad2(int value) {
    String s = String(value);
    if (s.length() < 2) {
        s = "0" + s;
    }
    return s;
}

String ClockManager::getFormattedTime() {
    int h = timeClient.getHours();   // 0-23, 24-hour format
    int m = timeClient.getMinutes();
    int s = timeClient.getSeconds();

    return pad2(h) + ":" + pad2(m) + ":" + pad2(s);
}
