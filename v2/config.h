#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
// WIFI CONFIGURATION
// ============================================================
#define WIFI_SSID       "antivirus"
#define WIFI_PASSWORD   "antivirus"

// Hostname the device will advertise on the network
#define DEVICE_HOSTNAME "watch-clock"

// ============================================================
// NTP CONFIGURATION
// ============================================================
#define NTP_SERVER          "pool.ntp.org"

// Offset of local time from UTC, in seconds.
// Example: India (UTC+5:30)  => 19800
//          UTC                => 0
//          US Eastern (UTC-5) => -18000
#define GMT_OFFSET_SEC      19800

// How often (ms) the internal NTPClient object is allowed to
// reach out to the server. The library itself throttles actual
// network traffic to this interval; update() is safe to call
// every loop() iteration.
#define NTP_UPDATE_INTERVAL_MS   60000UL   // library-level throttle (1 min)

// How often (ms) we force a *fresh* resync explicitly, per the
// "resync every hour" requirement.
#define NTP_RESYNC_INTERVAL_MS   3600000UL // 1 hour

// ============================================================
// LCD CONFIGURATION (16x2, parallel 4-bit interface)
// ------------------------------------------------------------
// Pins chosen deliberately avoid the ESP8266 boot-strapping
// pins (GPIO0/D3, GPIO2/D4, GPIO15/D8) so the module boots
// reliably regardless of LCD wiring state at power-up.
// ============================================================
#define LCD_RS_PIN   D1   // GPIO5
#define LCD_EN_PIN   D2   // GPIO4
#define LCD_D4_PIN   D5   // GPIO14
#define LCD_D5_PIN   D6   // GPIO12
#define LCD_D6_PIN   D7   // GPIO13
#define LCD_D7_PIN   D0   // GPIO16

#define LCD_COLS     16
#define LCD_ROWS     2

// How often the LCD is allowed to repaint, in milliseconds.
// The stopwatch needs hundredths-of-a-second resolution, but we
// still throttle actual LCD writes to keep the display readable
// and avoid unnecessary bus traffic.
#define LCD_REFRESH_INTERVAL_MS   100UL

// ============================================================
// SHARED TYPES
// ============================================================

// Which module's content is currently shown on the physical LCD.
// All three modules keep running regardless of this selection.
enum DisplayMode {
    CLOCK_MODE,
    TIMER_MODE,
    STOPWATCH_MODE
};

// Countdown timer state machine.
enum TimerState {
    TIMER_IDLE,
    TIMER_RUNNING,
    TIMER_PAUSED,
    TIMER_FINISHED
};

// Stopwatch state machine.
enum StopwatchState {
    STOPWATCH_IDLE,
    STOPWATCH_RUNNING,
    STOPWATCH_PAUSED
};

#endif // CONFIG_H
