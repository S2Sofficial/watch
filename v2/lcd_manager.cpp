#include "lcd_manager.h"

LCDManager::LCDManager()
    : lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN),
      currentLine1(""),
      currentLine2(""),
      firstRender(true),
      lastRefreshMillis(0)
{
}

void LCDManager::begin() {
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.clear(); // One-time clear at boot only.
    currentLine1 = "";
    currentLine2 = "";
    firstRender = true;
}

String LCDManager::fitToWidth(const String &text) {
    String result = text;
    if (result.length() > LCD_COLS) {
        result = result.substring(0, LCD_COLS);
    }
    while (result.length() < LCD_COLS) {
        result += ' ';
    }
    return result;
}

void LCDManager::writeLineIfChanged(uint8_t row, const String &newLine, String &cachedLine) {
    if (!firstRender && newLine == cachedLine) {
        return; // Nothing changed on this row; skip writing entirely.
    }

    // Only rewrite the individual characters that differ, rather than
    // clearing/rewriting the whole row, to minimize visible flicker.
    for (uint8_t col = 0; col < LCD_COLS; col++) {
        char oldCh = (col < cachedLine.length()) ? cachedLine[col] : '\0';
        char newCh = newLine[col];
        if (firstRender || oldCh != newCh) {
            lcd.setCursor(col, row);
            lcd.write((uint8_t)newCh);
        }
    }

    cachedLine = newLine;
}

void LCDManager::render(const String &line1, const String &line2) {
    unsigned long now = millis();
    if (!firstRender && (now - lastRefreshMillis < LCD_REFRESH_INTERVAL_MS)) {
        return; // Throttle: not time for another repaint yet.
    }
    lastRefreshMillis = now;

    String l1 = fitToWidth(line1);
    String l2 = fitToWidth(line2);

    writeLineIfChanged(0, l1, currentLine1);
    writeLineIfChanged(1, l2, currentLine2);

    firstRender = false;
}
