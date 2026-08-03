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

// How often (ms) loop() attempts a manual reconnect while Wi-Fi is
// known to be down. (The ESP8266 core auto-reconnects on its own in
// most cases; this is a non-blocking fallback + gives us a place to
// log the event.)
#define WIFI_RETRY_INTERVAL_MS   5000UL

// How long (ms) the LCD shows the "IP address" banner after boot or
// after reconnecting, before returning to the normal display.
#define WIFI_IP_BANNER_MS        4000UL

// ============================================================
// NTP CONFIGURATION
// ============================================================
#define NTP_SERVER          "pool.ntp.org"

// Offset of local time from UTC, in seconds.
// Example: India (UTC+5:30)  => 19800
//          UTC                => 0
//          US Eastern (UTC-5) => -18000
#define GMT_OFFSET_SEC      19800

// How often (ms) we force a *fresh* resync explicitly, per the
// "resync every hour" requirement.
#define NTP_RESYNC_INTERVAL_MS   3600000UL // 1 hour

// How often (ms) the internal NTPClient object is allowed to reach
// out to the server on its own. IMPORTANT: NTPClient::update() calls
// forceUpdate() once this interval elapses, and forceUpdate() BLOCKS
// the entire loop() (delay(10) up to 100 times, i.e. up to ~1s) while
// it waits for the UDP reply - longer still if a reply is lost and
// the full timeout is hit. With the old 60s value this caused a
// visible web-UI/LCD stall roughly once a minute. Since we already
// force an explicit resync on NTP_RESYNC_INTERVAL_MS below, the
// automatic update is only a safety net, so it's set to the same
// hourly cadence instead - time is still extrapolated from millis()
// between syncs, so accuracy is unaffected.
#define NTP_UPDATE_INTERVAL_MS   NTP_RESYNC_INTERVAL_MS

// ============================================================
// LCD CONFIGURATION (16x2 over I2C, e.g. PCF8574 backpack)
// ------------------------------------------------------------
// Only two wires (SDA/SCL) are needed now instead of six.
// On NodeMCU, the default ESP8266 Wire pins are:
//   SDA = D2 (GPIO4)
//   SCL = D1 (GPIO5)
// These are safe pins (not boot-strapping) and are used unless
// you change them below.
//
// LCD_I2C_ADDRESS depends on your backpack: 0x27 and 0x3F are
// the two most common. If the display stays blank/garbled, run
// an I2C scanner sketch to find the correct address before
// assuming it's a wiring problem.
// ============================================================
#define LCD_I2C_ADDRESS   0x27

#define LCD_SDA_PIN       D2   // GPIO4
#define LCD_SCL_PIN       D1   // GPIO5

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
