#ifndef STOPWATCH_MANAGER_H
#define STOPWATCH_MANAGER_H

#include <Arduino.h>
#include "config.h"

// Non-blocking stopwatch with hundredths-of-a-second resolution.
// Internal timing is derived entirely from millis().
class StopwatchManager {
public:
    StopwatchManager();

    // Call once per loop(). Cheap: just bookkeeping, no LCD/network I/O.
    void update();

    void start();   // IDLE -> RUNNING (fresh run from zero)
    void pause();   // RUNNING -> PAUSED
    void resume();  // PAUSED -> RUNNING
    void reset();   // any state -> IDLE, elapsed time cleared

    // Returns "MM:SS.hh" (hh = hundredths of a second).
    String getFormattedTime() const;

    // Total elapsed time in milliseconds, live while running.
    unsigned long getElapsedMillis() const;

    StopwatchState getState() const { return state; }
    bool isRunning() const { return state == STOPWATCH_RUNNING; }

private:
    StopwatchState state;

    unsigned long runStartMillis;     // millis() when the current run began
    unsigned long accumulatedMillis;  // time banked from previous runs (before pause)

    static String pad2(unsigned long value);
};

#endif // STOPWATCH_MANAGER_H
