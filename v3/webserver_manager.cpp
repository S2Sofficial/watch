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

void WebServerManager::lowLatency() {
    server.client().setNoDelay(true);
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
    lowLatency();
    server.send_P(200, "text/html", WATCH_WEBPAGE);
}

const char *WebServerManager::timerStateToString(TimerState s) {
    switch (s) {
        case TIMER_IDLE:     return "idle";
        case TIMER_RUNNING:  return "running";
        case TIMER_PAUSED:   return "paused";
        case TIMER_FINISHED: return "finished";
    }
    return "idle";
}

const char *WebServerManager::stopwatchStateToString(StopwatchState s) {
    switch (s) {
        case STOPWATCH_IDLE:    return "idle";
        case STOPWATCH_RUNNING: return "running";
        case STOPWATCH_PAUSED:  return "paused";
    }
    return "idle";
}

const char *WebServerManager::displayModeToString(DisplayMode m) {
    switch (m) {
        case CLOCK_MODE:     return "clock";
        case TIMER_MODE:     return "timer";
        case STOPWATCH_MODE: return "stopwatch";
    }
    return "clock";
}

void WebServerManager::handleStatus() {
    lowLatency();

    // Built into a fixed stack buffer (no ArduinoJson dependency, and
    // no chained String concatenation) since this endpoint is polled
    // every ~300ms for as long as the device is up - repeated String
    // "+=" growth here would slowly fragment the ESP8266's small heap
    // over hours/days of uptime and was a likely cause of the device
    // eventually becoming sluggish or unresponsive.
    char json[256];
    snprintf(json, sizeof(json),
        "{\"display\":\"%s\",\"clock\":\"%s\",\"timer\":\"%s\",\"timerRunning\":%s,"
        "\"timerState\":\"%s\",\"stopwatch\":\"%s\",\"stopwatchRunning\":%s,\"stopwatchState\":\"%s\"}",
        displayModeToString(currentDisplay),
        clockMgr.getFormattedTime().c_str(),
        timerMgr.getFormattedTime().c_str(),
        timerMgr.isRunning() ? "true" : "false",
        timerStateToString(timerMgr.getState()),
        stopwatchMgr.getFormattedTime().c_str(),
        stopwatchMgr.isRunning() ? "true" : "false",
        stopwatchStateToString(stopwatchMgr.getState()));

    server.send(200, "application/json", json);
}

void WebServerManager::handleDisplaySelect() {
    lowLatency();

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

    Serial.print(F("[Web] Display switched to: "));
    Serial.println(mode);

    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------
// Timer endpoints
// ------------------------------------------------------------------

void WebServerManager::handleTimerStart() {
    lowLatency();

    // Clamp rather than trust the raw input: toInt() on an out-of-range
    // or malformed value (e.g. "h=300") would otherwise silently wrap
    // when narrowed to uint8_t (300 -> 44), starting a very different
    // countdown than what was actually requested.
    long hRaw = server.hasArg("h") ? server.arg("h").toInt() : 0;
    long mRaw = server.hasArg("m") ? server.arg("m").toInt() : 0;
    long sRaw = server.hasArg("s") ? server.arg("s").toInt() : 0;
    uint8_t h = (uint8_t)constrain(hRaw, 0, 99);
    uint8_t m = (uint8_t)constrain(mRaw, 0, 59);
    uint8_t s = (uint8_t)constrain(sRaw, 0, 59);

    timerMgr.start(h, m, s);

    Serial.print(F("[Web] Timer start: "));
    Serial.print(h); Serial.print(':'); Serial.print(m); Serial.print(':'); Serial.println(s);

    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerPause() {
    lowLatency();
    timerMgr.pause();
    Serial.println(F("[Web] Timer pause"));
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerResume() {
    lowLatency();
    timerMgr.resume();
    Serial.println(F("[Web] Timer resume"));
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleTimerReset() {
    lowLatency();
    timerMgr.reset();
    Serial.println(F("[Web] Timer reset"));
    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------
// Stopwatch endpoints
// ------------------------------------------------------------------

void WebServerManager::handleStopwatchStart() {
    lowLatency();
    stopwatchMgr.start();
    Serial.println(F("[Web] Stopwatch start"));
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchPause() {
    lowLatency();
    stopwatchMgr.pause();
    Serial.println(F("[Web] Stopwatch pause"));
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchResume() {
    lowLatency();
    stopwatchMgr.resume();
    Serial.println(F("[Web] Stopwatch resume"));
    server.send(200, "text/plain", "OK");
}

void WebServerManager::handleStopwatchReset() {
    lowLatency();
    stopwatchMgr.reset();
    Serial.println(F("[Web] Stopwatch reset"));
    server.send(200, "text/plain", "OK");
}

// ------------------------------------------------------------------

void WebServerManager::handleNotFound() {
    lowLatency();
    server.send(404, "text/plain", "Not found");
}
