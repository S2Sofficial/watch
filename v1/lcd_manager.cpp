#include "lcd_manager.h"
#include "config.h"
#include "clock_manager.h"
#include "timer_manager.h"
#include "stopwatch_manager.h"

#include <LiquidCrystal.h>
#include <string.h>

namespace
{
    LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

    DisplayMode currentMode = CLOCK_MODE;

    // Shadow copies of what is currently on each LCD row, used to
    // diff against the next frame so only changed cells are rewritten.
    char lastLine0[LCD_COLS + 1];
    char lastLine1[LCD_COLS + 1];
    bool forceFullRedraw = true;

    unsigned long lastRefresh = 0;

    // Writes `newText` (padded/truncated to LCD_COLS) into `row`,
    // touching the LCD only at the character positions that differ
    // from what was drawn last time (or all of them if forced).
    void writeLineIfChanged(uint8_t row, const char *newText, char *lastText)
    {
        char padded[LCD_COLS + 1];
        size_t len = strlen(newText);

        for (uint8_t i = 0; i < LCD_COLS; i++)
        {
            padded[i] = (i < len) ? newText[i] : ' ';
        }
        padded[LCD_COLS] = '\0';

        for (uint8_t i = 0; i < LCD_COLS; i++)
        {
            if (forceFullRedraw || padded[i] != lastText[i])
            {
                lcd.setCursor(i, row);
                lcd.write((uint8_t)padded[i]);
            }
        }

        memcpy(lastText, padded, LCD_COLS + 1);
    }
}

void LcdManager::begin()
{
    lcd.begin(LCD_COLS, LCD_ROWS);
    memset(lastLine0, 0, sizeof(lastLine0));
    memset(lastLine1, 0, sizeof(lastLine1));
    forceFullRedraw = true;
    lcd.clear();
}

void LcdManager::setDisplayMode(DisplayMode mode)
{
    if (mode != currentMode)
    {
        currentMode = mode;
        forceRedraw();
    }
}

DisplayMode LcdManager::getDisplayMode()
{
    return currentMode;
}

void LcdManager::forceRedraw()
{
    forceFullRedraw = true;
}

void LcdManager::update()
{
    unsigned long now = millis();
    if (now - lastRefresh < LCD_REFRESH_INTERVAL_MS)
    {
        return;
    }
    lastRefresh = now;

    String title;
    String value;

    // Only the selected module's value is read here; all three modules
    // keep running in the background regardless of what is displayed.
    switch (currentMode)
    {
    case CLOCK_MODE:
        title = "Clock";
        value = ClockManager::getTimeString();
        break;
    case TIMER_MODE:
        title = "Timer";
        value = TimerManager::getTimeString();
        break;
    case STOPWATCH_MODE:
        title = "Stopwatch";
        value = StopwatchManager::getTimeString();
        break;
    }

    writeLineIfChanged(0, title.c_str(), lastLine0);
    writeLineIfChanged(1, value.c_str(), lastLine1);

    forceFullRedraw = false;
}
