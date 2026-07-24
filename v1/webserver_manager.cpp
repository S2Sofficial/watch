#include "webserver_manager.h"
#include "config.h"
#include "webpage.h"
#include "lcd_manager.h"
#include "clock_manager.h"
#include "timer_manager.h"
#include "stopwatch_manager.h"

#include <ESP8266WebServer.h>

namespace
{
    ESP8266WebServer server(HTTP_PORT);

    String displayModeToString(DisplayMode mode)
    {
        switch (mode)
        {
        case CLOCK_MODE:      return "clock";
        case TIMER_MODE:      return "timer";
        case STOPWATCH_MODE:  return "stopwatch";
        }
        return "clock";
    }

    // ---------------- GET / ----------------
    void handleRoot()
    {
        server.send_P(200, "text/html", INDEX_HTML);
    }

    // ---------------- GET /status ----------------
    // Returns the complete device status as JSON, matching the shape:
    // {"display":"timer","clock":"14:25:36","timer":"00:10:14",
    //  "timerRunning":true,"stopwatch":"03:25.47","stopwatchRunning":false}
    void handleStatus()
    {
        String json;
        json.reserve(200);

        json += "{";
        json += "\"display\":\"" + displayModeToString(LcdManager::getDisplayMode()) + "\",";
        json += "\"clock\":\"" + ClockManager::getTimeString() + "\",";
        json += "\"timer\":\"" + TimerManager::getTimeString() + "\",";
        json += "\"timerRunning\":" + String(TimerManager::isRunning() ? "true" : "false") + ",";
        json += "\"stopwatch\":\"" + StopwatchManager::getTimeString() + "\",";
        json += "\"stopwatchRunning\":" + String(StopwatchManager::isRunning() ? "true" : "false");
        json += "}";

        server.send(200, "application/json", json);
    }

    // ---------------- GET /display?mode=... ----------------
    // Only ever changes which module is shown on the LCD; never touches
    // the running/paused state of Clock, Timer or Stopwatch.
    void handleDisplaySelect()
    {
        if (server.hasArg("mode"))
        {
            String mode = server.arg("mode");
            if (mode == "clock")
            {
                LcdManager::setDisplayMode(CLOCK_MODE);
            }
            else if (mode == "timer")
            {
                LcdManager::setDisplayMode(TIMER_MODE);
            }
            else if (mode == "stopwatch")
            {
                LcdManager::setDisplayMode(STOPWATCH_MODE);
            }
        }
        server.send(200, "text/plain", "OK");
    }

    // ---------------- POST /timer/start ----------------
    // Optional hours/minutes/seconds query args configure a new
    // duration before starting; otherwise the last configured
    // duration is reused.
    void handleTimerStart()
    {
        if (server.hasArg("hours") || server.hasArg("minutes") || server.hasArg("seconds"))
        {
            uint8_t h = server.hasArg("hours")   ? (uint8_t)server.arg("hours").toInt()   : 0;
            uint8_t m = server.hasArg("minutes") ? (uint8_t)server.arg("minutes").toInt() : 0;
            uint8_t s = server.hasArg("seconds") ? (uint8_t)server.arg("seconds").toInt() : 0;
            TimerManager::setDuration(h, m, s);
        }
        TimerManager::start();
        server.send(200, "text/plain", "OK");
    }

    void handleTimerPause()
    {
        TimerManager::pause();
        server.send(200, "text/plain", "OK");
    }

    void handleTimerResume()
    {
        TimerManager::resume();
        server.send(200, "text/plain", "OK");
    }

    void handleTimerReset()
    {
        TimerManager::reset();
        server.send(200, "text/plain", "OK");
    }

    // ---------------- Stopwatch control endpoints ----------------
    void handleStopwatchStart()
    {
        StopwatchManager::start();
        server.send(200, "text/plain", "OK");
    }

    void handleStopwatchPause()
    {
        StopwatchManager::pause();
        server.send(200, "text/plain", "OK");
    }

    void handleStopwatchResume()
    {
        StopwatchManager::resume();
        server.send(200, "text/plain", "OK");
    }

    void handleStopwatchReset()
    {
        StopwatchManager::reset();
        server.send(200, "text/plain", "OK");
    }

    void handleNotFound()
    {
        server.send(404, "text/plain", "Not found");
    }
}

void WebServerManager::begin()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);

    server.on("/display", HTTP_GET, handleDisplaySelect);

    server.on("/timer/start", HTTP_POST, handleTimerStart);
    server.on("/timer/pause", HTTP_POST, handleTimerPause);
    server.on("/timer/resume", HTTP_POST, handleTimerResume);
    server.on("/timer/reset", HTTP_POST, handleTimerReset);

    server.on("/stopwatch/start", HTTP_POST, handleStopwatchStart);
    server.on("/stopwatch/pause", HTTP_POST, handleStopwatchPause);
    server.on("/stopwatch/resume", HTTP_POST, handleStopwatchResume);
    server.on("/stopwatch/reset", HTTP_POST, handleStopwatchReset);

    server.onNotFound(handleNotFound);

    server.begin();
}

void WebServerManager::handleClient()
{
    server.handleClient();
}
