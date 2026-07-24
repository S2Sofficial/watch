#include "webserver_manager.h"
#include "webpage.h"

WebServerManager::WebServerManager(ClockManager &clock,
                                    TimerManager &timer,
                                    StopwatchManager &stopwatch,
                                    DisplayMode &displayModeRef)
    : server(80),
      clockMgr(clock),
      timerMgr(timer),
      stopwatchMgr(stopwatch),
      currentDisplay(displayModeRef)
{
}

void WebServerManager::begin() {
    setupRoutes();
    server.begin();
    Serial.println(F("[Web] HTTP server started on port 80."));
}

void WebServerManager::handleClient() {
    server.handleClient(); // Non-blocking: services at most one request.
}

void WebServerManager::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/display", HTTP_GET, [this]() { handleDisplaySelect(); });

    server.on("/timer/start", HTTP_POST, [this]() { handleTimerStart(); });
    server.on("/timer/pause", HTTP_POST, [this]() { handleTimerPause(); });
    server.on("/timer/resume", HTTP_POST, [this]() { handleTimerResume(); });
    server.on("/timer/reset", HTTP_POST, [this]() { handleTimerReset(); });

    server.on("/stopwatch/start", HTTP_POST, [this]() { handleStopwatchStart(); });
    server.on("/stopwatch/pause", HTTP_POST, [this]() { handleStopwatchPause(); });
    server.on("/stopwatch/resume", HTTP_POST, [this]() { handleStopwatchResume(); });
    server.on("/stopwatch/reset", HTTP_POST, [this]() { handleStopwatchReset(); });

    server.onNotFound([this]() { handleNotFound(); });
}

// ------------------------------------------------------------------
// Page / status
// ------------------------------------------------------------------

void WebServerManager::handleRoot() {
    server.send_P(200, "text/html", WATCH_WEBPAGE);
}

String WebServerManager::timerStateToString(TimerState s) {
    switch (s) {
        case TIMER_IDLE:     return "idle";
        case TIMER_RUNNING:  return "running";
        case TIMER_PAUSED:   return "paused";
        case TIMER_FINISHED: return "finished";
    }
    return "idle";
}

String WebServerManager::stopwatchStateToString(StopwatchState s) {
    switch (s) {
        case STOPWATCH_IDLE:    return "idle";
        case STOPWATCH_RUNNING: return "running";
        case STOPWATCH_PAUSED:  return "paused";
    }
    return "idle";
}

String WebServerManager::displayModeToString(DisplayMode m) {
    switch (m) {
        case CLOCK_MODE:     return "clock";
        case TIMER_MODE:     return "timer";
        case STOPWATCH_MODE: return "stopwatch";
    }
    return "clock";
}

void WebServerManager::handleStatus() {
    // Built manually (no ArduinoJson dependency) to keep the project
    // on the standard library set requested.
    String json = "{";
    json += "\"display\":\"" + displayModeToString(currentDisplay) + "\",";
    json += "\"clock\":\"" + clockMgr.getFormattedTime() + "\",";
    json += "\"timer\":\"" + timerMgr.getFormattedTime() + "\",";
    json += "\"timerRunning\":" + String(timerMgr.isRunning() ? "true" : "false") + ",";
    json += "\"timerState\":\"" + timerStateToString(timerMgr.getState()) + "\",";
    json += "\"stopwatch\":\"" + stopwatchMgr.getFormattedTime() + "\",";
    json += "\"stopwatchRunning\":" + String(stopwatchMgr.isRunning() ? "true" : "false") + ",";
    json += "\"stopwatchState\":\"" + stopwatchStateToString(stopwatchMgr.getState()) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void WebServerManager::handleDisplaySelect() {
    if (!server.hasArg("mode")) {
        server.send(400, "text/plain", "Missing 'mode' argument");
        return;
    }

    String mode = server.arg("mode");
    if (mode == "clock") {
        currentDisplay = CLOCK_MODE;
    } else if (mode == "timer") {
        currentDisplay = TIMER_MODE;
    } else if (mode == "stopwatch") {
        currentDisplay = STOPWATCH_MODE;
    } else {
        server.send(400, "text/plain", "Invalid mode");
        return;
    }

    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------
// Timer endpoints
// ------------------------------------------------------------------

void WebServerManager::handleTimerStart() {
    uint8_t h = server.hasArg("h") ? (uint8_t)server.arg("h").toInt() : 0;
    uint8_t m = server.hasArg("m") ? (uint8_t)server.arg("m").toInt() : 0;
    uint8_t s = server.hasArg("s") ? (uint8_t)server.arg("s").toInt() : 0;

    timerMgr.start(h, m, s);
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerPause() {
    timerMgr.pause();
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerResume() {
    timerMgr.resume();
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerReset() {
    timerMgr.reset();
    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------
// Stopwatch endpoints
// ------------------------------------------------------------------

void WebServerManager::handleStopwatchStart() {
    stopwatchMgr.start();
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchPause() {
    stopwatchMgr.pause();
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchResume() {
    stopwatchMgr.resume();
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchReset() {
    stopwatchMgr.reset();
    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------

void WebServerManager::handleNotFound() {
    server.send(404, "text/plain", "Not found");
}
