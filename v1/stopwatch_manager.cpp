#include "stopwatch_manager.h"

namespace
{
    StopwatchState state = StopwatchState::IDLE;

    unsigned long accumulatedMs = 0;   // total elapsed ms across all completed RUNNING segments
    unsigned long runStartMillis = 0;  // millis() when the current RUNNING segment began
}

void StopwatchManager::begin()
{
    state = StopwatchState::IDLE;
    accumulatedMs = 0;
}

void StopwatchManager::start()
{
    if (state == StopwatchState::IDLE)
    {
        accumulatedMs = 0;
        runStartMillis = millis();
        state = StopwatchState::RUNNING;
    }
}

void StopwatchManager::pause()
{
    if (state == StopwatchState::RUNNING)
    {
        accumulatedMs += millis() - runStartMillis;
        state = StopwatchState::PAUSED;
    }
}

void StopwatchManager::resume()
{
    if (state == StopwatchState::PAUSED)
    {
        runStartMillis = millis();
        state = StopwatchState::RUNNING;
    }
}

void StopwatchManager::reset()
{
    state = StopwatchState::IDLE;
    accumulatedMs = 0;
}

void StopwatchManager::update()
{
    // Elapsed time is always derived on demand from millis() in
    // getTimeString(), so there is nothing to tick here. The function
    // exists to keep the module's calling convention consistent with
    // ClockManager/TimerManager inside the main loop.
}

String StopwatchManager::getTimeString()
{
    unsigned long elapsed = accumulatedMs;
    if (state == StopwatchState::RUNNING)
    {
        elapsed += millis() - runStartMillis;
    }

    unsigned long totalHundredths = elapsed / 10UL;
    unsigned long minutes = totalHundredths / 6000UL;
    unsigned long seconds = (totalHundredths / 100UL) % 60UL;
    unsigned long hundredths = totalHundredths % 100UL;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu.%02lu", minutes, seconds, hundredths);
    return String(buf);
}

StopwatchState StopwatchManager::getState()
{
    return state;
}

bool StopwatchManager::isRunning()
{
    return state == StopwatchState::RUNNING;
}
