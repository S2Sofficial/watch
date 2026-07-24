#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>

enum class TimerState
{
    IDLE,
    RUNNING,
    PAUSED,
    FINISHED
};

namespace TimerManager
{
    void begin();

    // Call every loop() iteration. Decrements the remaining time by
    // whole seconds based on millis(), never blocking.
    void update();

    // Sets (or replaces) the countdown duration. Also resets the
    // remaining time to the new duration and returns to IDLE.
    void setDuration(uint8_t hours, uint8_t minutes, uint8_t seconds);

    void start();   // begins counting down from the configured duration
    void pause();   // freezes the countdown, resumable later
    void resume();  // continues from where it was paused
    void reset();   // stops and restores remaining time to the full duration

    String getTimeString();   // remaining time as "HH:MM:SS"
    TimerState getState();
    bool isRunning();
}

#endif // TIMER_MANAGER_H
