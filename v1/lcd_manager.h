#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h>

// The single display mode the LCD can be in at any time.
// The web interface only ever changes this selector; it never
// touches the underlying Clock / Timer / Stopwatch state.
enum DisplayMode
{
    CLOCK_MODE,
    TIMER_MODE,
    STOPWATCH_MODE
};

namespace LcdManager
{
    // Initializes the LiquidCrystal driver and clears the display once.
    void begin();

    // Changes which module is shown on the LCD. Does not affect any
    // module's internal running/paused state.
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode();

    // Call every loop() iteration. Internally rate-limited by
    // LCD_REFRESH_INTERVAL_MS and only rewrites characters that changed.
    void update();

    // Forces a full clear + redraw on the next update() call
    // (used automatically on a mode switch).
    void forceRedraw();
}

#endif // LCD_MANAGER_H
