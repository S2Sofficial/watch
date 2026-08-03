#include "timer_manager.h"

TimerManager::TimerManager()
    : state(TIMER_IDLE),
      configuredSeconds(0),
      remainingSeconds(0),
      lastTickMillis(0)
{
}

void TimerManager::start(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    configuredSeconds = (long)hours * 3600L + (long)minutes * 60L + (long)seconds;
    remainingSeconds = configuredSeconds;
    lastTickMillis = millis();

    if (configuredSeconds > 0) {
        state = TIMER_RUNNING;
    } else {
        // Nothing to count down; treat as already finished.
        state = TIMER_FINISHED;
    }
}

void TimerManager::pause() {
    if (state == TIMER_RUNNING) {
        state = TIMER_PAUSED;
    }
}

void TimerManager::resume() {
    if (state == TIMER_PAUSED) {
        lastTickMillis = millis(); // avoid counting the paused interval
        state = TIMER_RUNNING;
    }
}

void TimerManager::reset() {
    remainingSeconds = configuredSeconds;
    state = TIMER_IDLE;
}

void TimerManager::update() {
    if (state != TIMER_RUNNING) {
        return;
    }

    unsigned long now = millis();
    unsigned long elapsed = now - lastTickMillis; // handles millis() overflow correctly

    if (elapsed >= 1000UL) {
        long ticks = (long)(elapsed / 1000UL);
        // Advance lastTickMillis by whole seconds consumed, keeping the
        // leftover sub-second remainder so we don't accumulate drift.
        lastTickMillis += (unsigned long)ticks * 1000UL;

        remainingSeconds -= ticks;
        if (remainingSeconds <= 0) {
            remainingSeconds = 0;
            state = TIMER_FINISHED;
        }
    }
}

String TimerManager::pad2(long value) {
    String s = String(value);
    if (s.length() < 2) {
        s = "0" + s;
    }
    return s;
}

String TimerManager::getFormattedTime() const {
    long h = remainingSeconds / 3600L;
    long m = (remainingSeconds % 3600L) / 60L;
    long s = remainingSeconds % 60L;
    return pad2(h) + ":" + pad2(m) + ":" + pad2(s);
}
