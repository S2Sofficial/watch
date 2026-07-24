#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Non-blocking countdown timer with one-second resolution.
// All timing is derived from millis(); no delay() is used.
class TimerManager {
public:
    TimerManager();

    // Call once per loop(). Decrements the countdown when RUNNING.
    void update();

    // Configures the countdown duration (in HH:MM:SS) and starts it.
    // Valid from IDLE, PAUSED, or FINISHED - always begins a fresh run.
    void start(uint8_t hours, uint8_t minutes, uint8_t seconds);

    // Pauses a RUNNING timer. No-op otherwise.
    void pause();

    // Resumes a PAUSED timer. No-op otherwise.
    void resume();

    // Stops the timer and restores the remaining time to the last
    // configured duration, returning to IDLE.
    void reset();

    // Returns "HH:MM:SS" for the remaining time.
    String getFormattedTime() const;

    TimerState getState() const { return state; }
    bool isRunning() const { return state == TIMER_RUNNING; }

private:
    TimerState state;

    long configuredSeconds;   // Total duration last set by the user
    long remainingSeconds;    // Seconds left on the countdown

    unsigned long lastTickMillis; // millis() at the last whole-second tick

    static String pad2(long value);
};

#endif // TIMER_MANAGER_H
