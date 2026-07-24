/*
  Watch
  ------------------------------------------------------------
  ESP8266 (NodeMCU) Wi-Fi Clock, Timer and Stopwatch

  - Clock time comes from NTP, synced at boot and hourly after, interpolated internally with millis() between syncs.
  - Countdown Timer and Stopwatch each run independently in the background using their own millis()-based state machines.
  - A 16x2 character LCD (4-bit parallel interface) shows exactly one of the three modules at a time, selected from the web UI. Switching the LCD's display never pauses or resets a module.
  - A single-page web interface (served from webpage.h) controls the Timer/Stopwatch and the LCD display selector via AJAX, polling /status roughly every 300ms.
  - The main loop is fully non-blocking: no delay() is used anywhere in this project.

  File layout:
    Watch.ino                 - setup()/loop(), wiring everything together
    config.h                  - WiFi, NTP, LCD pin and refresh settings
    webpage.h                 - single-page AJAX web UI (PROGMEM)
    lcd_manager.h/.cpp         - flicker-free partial LCD rendering
    clock_manager.h/.cpp       - NTP-synced wall clock
    timer_manager.h/.cpp       - countdown timer state machine
    stopwatch_manager.h/.cpp   - stopwatch state machine
    webserver_manager.h/.cpp   - HTTP routes / JSON status endpoint
*/

#include <ESP8266WiFi.h>

#include "config.h"
#include "lcd_manager.h"
#include "clock_manager.h"
#include "timer_manager.h"
#include "stopwatch_manager.h"
#include "webserver_manager.h"

namespace
{
    void connectWiFi()
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        Serial.print("Connecting to WiFi");

        unsigned long startAttempt = millis();
        unsigned long lastDot = 0;

        // Bounded wait using yield() instead of delay(): keeps the
        // WiFi/TCP stack serviced while giving the network up to
        // 30 seconds to associate before falling through.
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 30000UL)
        {
            yield();
            if (millis() - lastDot >= 500UL)
            {
                Serial.print(".");
                lastDot = millis();
            }
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("Connected. IP address: ");
            Serial.println(WiFi.localIP());
        }
        else
        {
            Serial.println("WiFi not connected yet - the ESP8266 will keep retrying in the background.");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\nWatch - ESP8266 Clock / Timer / Stopwatch");

    connectWiFi();

    LcdManager::begin();
    ClockManager::begin();
    TimerManager::begin();
    StopwatchManager::begin();
    WebServerManager::begin();

    // Default LCD view on boot.
    LcdManager::setDisplayMode(CLOCK_MODE);
}

void loop()
{
    // Service any pending HTTP requests.
    WebServerManager::handleClient();

    // Advance each independent module. All three always run,
    // regardless of which one is currently shown on the LCD.
    ClockManager::update();
    TimerManager::update();
    StopwatchManager::update();

    // Redraw only the characters that changed on the LCD, for
    // whichever module is currently selected.
    LcdManager::update();
}
