#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "clock_manager.h"
#include "timer_manager.h"
#include "stopwatch_manager.h"

// Owns the ESP8266WebServer instance, serves the single-page UI,
// and exposes the REST/JSON endpoints. Holds pointers to the other
// managers so it can read their state and forward commands, but does
// not own or update their timing logic itself.
class WebServerManager {
public:
    WebServerManager(ClockManager &clock,
                      TimerManager &timer,
                      StopwatchManager &stopwatch,
                      DisplayMode &displayModeRef);

    void begin();

    // Non-blocking. Call every loop() iteration.
    void handleClient();

private:
    ESP8266WebServer server;

    ClockManager &clockMgr;
    TimerManager &timerMgr;
    StopwatchManager &stopwatchMgr;
    DisplayMode &currentDisplay;

    void setupRoutes();

    // Route handlers
    void handleRoot();
    void handleStatus();
    void handleDisplaySelect();

    void handleTimerStart();
    void handleTimerPause();
    void handleTimerResume();
    void handleTimerReset();

    void handleStopwatchStart();
    void handleStopwatchPause();
    void handleStopwatchResume();
    void handleStopwatchReset();

    void handleNotFound();

    static String timerStateToString(TimerState s);
    static String stopwatchStateToString(StopwatchState s);
    static String displayModeToString(DisplayMode m);
};

#endif // WEBSERVER_MANAGER_H
