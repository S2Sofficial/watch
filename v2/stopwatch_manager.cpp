#include "stopwatch_manager.h"

StopwatchManager::StopwatchManager()
    : state(STOPWATCH_IDLE),
      runStartMillis(0),
      accumulatedMillis(0)
{
}

void StopwatchManager::start() {
    accumulatedMillis = 0;
    runStartMillis = millis();
    state = STOPWATCH_RUNNING;
}

void StopwatchManager::pause() {
    if (state == STOPWATCH_RUNNING) {
        accumulatedMillis += millis() - runStartMillis;
        state = STOPWATCH_PAUSED;
    }
}

void StopwatchManager::resume() {
    if (state == STOPWATCH_PAUSED) {
        runStartMillis = millis();
        state = STOPWATCH_RUNNING;
    }
}

void StopwatchManager::reset() {
    accumulatedMillis = 0;
    runStartMillis = millis();
    state = STOPWATCH_IDLE;
}

void StopwatchManager::update() {
    // No periodic bookkeeping is required: getElapsedMillis() computes
    // the live value on demand from millis(), so there is nothing to
    // advance here. The method exists to keep a uniform manager
    // interface (update() called every loop iteration) and as a place
    // for future housekeeping (e.g. overflow handling) if needed.
}

unsigned long StopwatchManager::getElapsedMillis() const {
    if (state == STOPWATCH_RUNNING) {
        return accumulatedMillis + (millis() - runStartMillis);
    }
    return accumulatedMillis;
}

String StopwatchManager::pad2(unsigned long value) {
    String s = String(value);
    if (s.length() < 2) {
        s = "0" + s;
    }
    return s;
}

String StopwatchManager::getFormattedTime() const {
    unsigned long totalMs = getElapsedMillis();

    unsigned long minutes = totalMs / 60000UL;
    unsigned long seconds = (totalMs / 1000UL) % 60UL;
    unsigned long hundredths = (totalMs % 1000UL) / 10UL;

    // Minutes can exceed 2 digits for very long runs; only seconds and
    // hundredths are strictly padded to 2 digits per the MM:SS.hh spec.
    String mm = String(minutes);
    if (mm.length() < 2) mm = "0" + mm;

    return mm + ":" + pad2(seconds) + "." + pad2(hundredths);
}
