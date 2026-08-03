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
    // NOTE: when NTPClient's internal update interval has elapsed,
    // update() calls forceUpdate() internally, which BLOCKS for up to
    // ~1s waiting on the UDP reply (see NTP_UPDATE_INTERVAL_MS comment
    // in config.h for why that interval is set the way it is). Most
    // loop() iterations this call returns immediately having just
    // extrapolated the time from millis().
    if (timeClient.update()) {
        synced = true;
    }

    // Explicit forced resync on the required schedule. Same blocking
    // caveat as above applies here, just once an hour instead of
    // silently every minute.
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

void ClockManager::forceResyncSoon() {
    // Rewind lastResyncMillis so the next update() call's elapsed-time
    // check triggers an immediate forced resync (handles overflow the
    // same way update() does, since it's the same unsigned subtraction).
    lastResyncMillis = millis() - NTP_RESYNC_INTERVAL_MS;
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
