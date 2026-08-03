/*
 * ============================================================
 *  Watch
 *  ESP8266 (NodeMCU) Wi-Fi Clock, Timer and Stopwatch
 * ============================================================
 *
 *  - Clock:      NTP-synchronized, 24-hour HH:MM:SS, resyncs hourly.
 *  - Timer:      Countdown set from the web UI, HH:MM:SS resolution.
 *  - Stopwatch:  millis()-based, MM:SS.hh (hundredths) resolution.
 *
 *  All three modules run continuously and independently in the
 *  background. The 16x2 LCD only ever shows ONE of them at a time,
 *  selected from the web interface; switching the display never
 *  pauses or resets the modules that are not currently shown.
 *
 *  The main loop is fully non-blocking: no delay() is used outside
 *  of the one-time NTP bootstrap in setup().
 *
 *  Required libraries (install via Library Manager):
 *    - ESP8266WiFi        (bundled with ESP8266 board package)
 *    - ESP8266WebServer    (bundled with ESP8266 board package)
 *    - WiFiUdp             (bundled with ESP8266 board package)
 *    - NTPClient           by Fabrice Weinberg
 *    - LiquidCrystal_I2C   by Frank de Brabander (or compatible fork)
 *    - Wire                (bundled with Arduino core, for I2C)
 * ============================================================
 */

#include <ESP8266WiFi.h>

#include "config.h"
#include "lcd_manager.h"
#include "clock_manager.h"
#include "timer_manager.h"
#include "stopwatch_manager.h"
#include "webserver_manager.h"

// ------------------------------------------------------------------
// Global state and module instances
// ------------------------------------------------------------------

// Which module is currently shown on the physical LCD. Changing this
// value is the ONLY effect the web interface's display selector has;
// it never touches the underlying timer/stopwatch/clock state.
DisplayMode currentDisplay = CLOCK_MODE;

LCDManager        lcdManager;
ClockManager      clockManager;
TimerManager      timerManager;
StopwatchManager  stopwatchManager;
WebServerManager  webServerManager(clockManager, timerManager, stopwatchManager, currentDisplay);

// Wi-Fi link monitoring (checked every loop(), never blocks).
bool wifiWasConnected = false;
unsigned long lastWifiRetryMillis = 0;

// While millis() < this, the LCD shows an "IP address" banner instead
// of the normal display (set at boot and again after every reconnect,
// so the address needed to reach the web UI is always visible without
// a serial monitor).
unsigned long ipBannerUntilMillis = 0;

// ------------------------------------------------------------------
// Wi-Fi bootstrap (setup()-time only; blocking here is acceptable
// since the device cannot do anything useful before it has network
// connectivity for NTP and the web UI).
// ------------------------------------------------------------------
void connectToWiFi() {
    WiFi.mode(WIFI_STA);

    // The ESP8266's default WiFi modem sleep saves power by dozing the
    // radio between beacon intervals, but that routinely adds 100ms+
    // of latency to every request and is one of the most common causes
    // of a "choppy"/unresponsive ESP8266 web server. This device is
    // always mains/USB powered, so trade that power saving for
    // consistently fast, responsive control from the web UI.
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    WiFi.hostname(DEVICE_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print(F("[WiFi] Connecting to "));
    Serial.print(WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED) {
        delay(300); // setup()-time only.
        Serial.print('.');
    }

    Serial.println();
    Serial.print(F("[WiFi] Connected. IP address: "));
    Serial.println(WiFi.localIP());

    wifiWasConnected = true;
    ipBannerUntilMillis = millis() + WIFI_IP_BANNER_MS;
}

// ------------------------------------------------------------------
// Non-blocking Wi-Fi link monitor. Detects loss/recovery of the
// connection, logs the transition to Serial, keeps retrying in the
// background, and drives the LCD's "Wi-Fi Lost" banner so a failure
// is visible even without a serial monitor attached.
// ------------------------------------------------------------------
void checkWiFiStatus() {
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    if (wifiWasConnected && !nowConnected) {
        Serial.println(F("[WiFi] Connection lost."));
        wifiWasConnected = false;
        lastWifiRetryMillis = millis();
    } else if (!wifiWasConnected && nowConnected) {
        Serial.print(F("[WiFi] Reconnected. IP address: "));
        Serial.println(WiFi.localIP());
        wifiWasConnected = true;
        ipBannerUntilMillis = millis() + WIFI_IP_BANNER_MS;
        clockManager.forceResyncSoon(); // clock may have drifted while offline
    } else if (!nowConnected) {
        unsigned long now = millis();
        if (now - lastWifiRetryMillis >= WIFI_RETRY_INTERVAL_MS) {
            lastWifiRetryMillis = now;
            Serial.println(F("[WiFi] Still disconnected, retrying..."));
            WiFi.reconnect(); // non-blocking; core also auto-reconnects on its own
        }
    }
}

// ------------------------------------------------------------------
// Per-module update wrappers (kept thin; real logic lives in each
// manager class to keep responsibilities separated).
// ------------------------------------------------------------------

void handleWebServer() {
    webServerManager.handleClient();
}

void updateClock() {
    clockManager.update();
}

void updateTimer() {
    timerManager.update();
}

void updateStopwatch() {
    stopwatchManager.update();
}

void updateLCD() {
    // Wi-Fi loss is the one failure that makes the web UI completely
    // unreachable, so it always takes priority over whatever module is
    // currently selected - the underlying clock/timer/stopwatch state
    // keeps running untouched regardless of what's shown here.
    if (WiFi.status() != WL_CONNECTED) {
        lcdManager.render("Wifi Lost!", "Reconnecting...");
        return;
    }

    if (millis() < ipBannerUntilMillis) {
        lcdManager.render("IP Address", WiFi.localIP().toString());
        return;
    }

    String line1;
    String line2;

    switch (currentDisplay) {
        case CLOCK_MODE:
            line1 = "Clock";
            // Before the first successful sync, NTPClient reports time
            // extrapolated from epoch 0, i.e. a plausible-looking but
            // completely wrong value - show that we're still syncing
            // instead of a confident, incorrect time.
            line2 = clockManager.isSynced() ? clockManager.getFormattedTime() : "Syncing...";
            break;

        case TIMER_MODE:
            line1 = "Timer";
            line2 = timerManager.getFormattedTime();
            break;

        case STOPWATCH_MODE:
            line1 = "Stopwatch";
            line2 = stopwatchManager.getFormattedTime();
            break;
    }

    lcdManager.render(line1, line2);
}

// ------------------------------------------------------------------
// Arduino entry points
// ------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("[Watch] Booting..."));

    lcdManager.begin();

    connectToWiFi();

    clockManager.begin();      // One-time blocking NTP sync happens here.
    webServerManager.begin();

    Serial.println(F("[Watch] Setup complete. Entering main loop."));
}

void loop() {
    checkWiFiStatus();

    handleWebServer();

    updateClock();
    updateTimer();
    updateStopwatch();

    updateLCD();

    // Deliberately no delay() here: every manager's update()/render()
    // call is internally throttled or O(1), so a tight loop is safe
    // and keeps the web server and LCD both responsive.
}
