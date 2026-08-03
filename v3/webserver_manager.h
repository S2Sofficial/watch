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

    // Disables Nagle's algorithm on the client socket for the request
    // currently being handled. Without this, ESP8266WebServer responses
    // interacting with delayed-ACK on the browser side routinely add
    // 200ms+ of latency per request, which is the main reason quick
    // button presses / status polls from the web UI can feel choppy or
    // unresponsive. Called first thing in every handler below.
    void lowLatency();

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

    // Return literal strings (not String) since this is on the hot
    // path of the /status endpoint, polled every ~300ms by the web UI;
    // avoiding avoidable heap churn there matters on an ESP8266.
    static const char *timerStateToString(TimerState s);
    static const char *stopwatchStateToString(StopwatchState s);
    static const char *displayModeToString(DisplayMode m);
};

#endif // WEBSERVER_MANAGER_H
