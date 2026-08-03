#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// Thin wrapper around LiquidCrystal_I2C that redraws only the
// characters which actually changed, avoiding lcd.clear() calls
// (which cause a visible flicker on every refresh) while still
// refreshing at a practical rate.
class LCDManager {
public:
    LCDManager();

    void begin();

    // Renders the given two lines (each must be <= LCD_COLS chars;
    // longer strings are truncated, shorter ones space-padded so
    // leftover characters from a previous longer string are erased).
    // Internally throttled to LCD_REFRESH_INTERVAL_MS and only writes
    // characters that differ from what is already on screen.
    void render(const String &line1, const String &line2);

private:
    LiquidCrystal_I2C lcd;

    String currentLine1;
    String currentLine2;
    bool firstRender;

    unsigned long lastRefreshMillis;

    static String fitToWidth(const String &text);
    void writeLineIfChanged(uint8_t row, const String &newLine, String &cachedLine);
};

#endif // LCD_MANAGER_H
