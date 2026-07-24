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
 *    - LiquidCrystal       (bundled with Arduino core)
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

// ------------------------------------------------------------------
// Wi-Fi bootstrap (setup()-time only; blocking here is acceptable
// since the device cannot do anything useful before it has network
// connectivity for NTP and the web UI).
// ------------------------------------------------------------------
void connectToWiFi() {
    WiFi.mode(WIFI_STA);
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
    String line1;
    String line2;

    switch (currentDisplay) {
        case CLOCK_MODE:
            line1 = "Clock";
            line2 = clockManager.getFormattedTime();
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
    handleWebServer();

    updateClock();
    updateTimer();
    updateStopwatch();

    updateLCD();

    // Deliberately no delay() here: every manager's update()/render()
    // call is internally throttled or O(1), so a tight loop is safe
    // and keeps the web server and LCD both responsive.
}
