#include "timer_manager.h"

namespace
{
    TimerState state = TimerState::IDLE;

    unsigned long totalDurationSec = 0;  // last configured duration
    unsigned long remainingSec = 0;      // seconds left in the countdown

    unsigned long lastTickMillis = 0;    // millis() at the last whole-second decrement
}

void TimerManager::begin()
{
    state = TimerState::IDLE;
    totalDurationSec = 0;
    remainingSec = 0;
}

void TimerManager::setDuration(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    totalDurationSec = (unsigned long)hours * 3600UL +
                        (unsigned long)minutes * 60UL +
                        (unsigned long)seconds;
    remainingSec = totalDurationSec;
    state = TimerState::IDLE;
}

void TimerManager::start()
{
    if (totalDurationSec == 0)
    {
        return; // nothing configured to count down
    }

    if (state == TimerState::IDLE || state == TimerState::FINISHED)
    {
        remainingSec = totalDurationSec;
    }

    state = TimerState::RUNNING;
    lastTickMillis = millis();
}

void TimerManager::pause()
{
    if (state == TimerState::RUNNING)
    {
        state = TimerState::PAUSED;
    }
}

void TimerManager::resume()
{
    if (state == TimerState::PAUSED)
    {
        state = TimerState::RUNNING;
        lastTickMillis = millis();
    }
}

void TimerManager::reset()
{
    state = TimerState::IDLE;
    remainingSec = totalDurationSec;
}

void TimerManager::update()
{
    if (state != TimerState::RUNNING)
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastTickMillis < 1000UL)
    {
        return;
    }

    unsigned long elapsedTicks = (now - lastTickMillis) / 1000UL;
    lastTickMillis += elapsedTicks * 1000UL;

    if (elapsedTicks >= remainingSec)
    {
        remainingSec = 0;
        state = TimerState::FINISHED;
    }
    else
    {
        remainingSec -= elapsedTicks;
    }
}

String TimerManager::getTimeString()
{
    unsigned long h = remainingSec / 3600UL;
    unsigned long m = (remainingSec % 3600UL) / 60UL;
    unsigned long s = remainingSec % 60UL;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);
    return String(buf);
}

TimerState TimerManager::getState()
{
    return state;
}

bool TimerManager::isRunning()
{
    return state == TimerState::RUNNING;
}
