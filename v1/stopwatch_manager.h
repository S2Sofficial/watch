#ifndef STOPWATCH_MANAGER_H
#define STOPWATCH_MANAGER_H

#include <Arduino.h>

enum class StopwatchState
{
    IDLE,
    RUNNING,
    PAUSED
};

namespace StopwatchManager
{
    void begin();

    // Present for architectural symmetry with the other modules;
    // elapsed time is computed on demand from millis(), so there is
    // no per-tick bookkeeping required here.
    void update();

    void start();   // starts counting up from zero
    void pause();   // freezes the elapsed time, resumable later
    void resume();  // continues accumulating from where it was paused
    void reset();   // stops and clears the elapsed time to zero

    String getTimeString();   // elapsed time as "MM:SS.hh" (hundredths)
    StopwatchState getState();
    bool isRunning();
}

#endif // STOPWATCH_MANAGER_H
