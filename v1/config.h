#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================================================
//  Wi-Fi Credentials  (Station mode only)
// =========================================================
#define WIFI_SSID     "antivirus"
#define WIFI_PASSWORD "antivirus"

// =========================================================
//  NTP Configuration
// =========================================================
#define NTP_SERVER              "pool.ntp.org"
#define NTP_UPDATE_INTERVAL_MS  3600000UL          // Resync every 1 hour
#define NTP_INITIAL_TIMEOUT_MS  15000UL            // Max time to wait for first sync at boot

// Timezone offset in seconds. Default below is IST (UTC+5:30).
// Change to match your location, e.g. 0 for UTC, -18000 for US Eastern (UTC-5).
#define GMT_OFFSET_SEC          (5 * 3600 + 30 * 60)

// =========================================================
//  LCD Pin Configuration (16x2, 4-bit parallel interface)
//  NodeMCU Dxx labels shown; rewire to match your board if different.
// =========================================================
#define LCD_RS   D1     // GPIO5
#define LCD_EN   D2     // GPIO4
#define LCD_D4   D5     // GPIO14
#define LCD_D5   D6     // GPIO12
#define LCD_D6   D7     // GPIO13
#define LCD_D7   D0     // GPIO16

#define LCD_COLS 16
#define LCD_ROWS 2

// How often the LCD content is recomputed/redrawn.
// 100 ms keeps the stopwatch's hundredths reasonably live without flicker.
#define LCD_REFRESH_INTERVAL_MS 100

// =========================================================
//  Web Server
// =========================================================
#define HTTP_PORT 80

#endif // CONFIG_H
