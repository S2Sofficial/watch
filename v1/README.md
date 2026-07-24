# Watch

ESP8266 (NodeMCU) Wi-Fi Clock, Timer and Stopwatch with a 16×2 parallel LCD
and a live web control panel. Fully non-blocking (no `delay()` in `loop()`).

## Hardware

- ESP8266 NodeMCU (USB powered)
- 16×2 character LCD, HD44780-compatible, wired in **4-bit parallel** mode (no I²C backpack, no RTC, no buttons, no buzzer)

### Wiring (NodeMCU → LCD)

| LCD pin | NodeMCU pin | GPIO |
|---------|-------------|------|
| RS      | D1          | GPIO5  |
| E       | D2          | GPIO4  |
| D4      | D5          | GPIO14 |
| D5      | D6          | GPIO12 |
| D6      | D7          | GPIO13 |
| D7      | D0          | GPIO16 |
| RW      | GND (tie low) | — |
| VSS     | GND         | — |
| VDD     | 5V (from USB) | — |
| V0      | Contrast pot wiper (10kΩ pot between VDD/GND) | — |
| A / K   | Backlight +5V / GND (via resistor if needed) | — |

Adjust the pin `#define`s in `config.h` if your wiring differs.

## Libraries required

Install via the Arduino Library Manager (Boards Manager: **esp8266** by ESP8266 Community):

- `ESP8266WiFi` (bundled with the esp8266 core)
- `ESP8266WebServer` (bundled with the esp8266 core)
- `WiFiUdp` (bundled with the esp8266 core)
- `NTPClient` (by Fabrice Weinberg)
- `LiquidCrystal` (bundled with Arduino IDE)

## Configuration

Edit `config.h` before flashing:

- `WIFI_SSID` / `WIFI_PASSWORD` — your network credentials
- `GMT_OFFSET_SEC` — your timezone offset in seconds (default: IST, UTC+5:30)
- `NTP_SERVER` / `NTP_UPDATE_INTERVAL_MS` — NTP host and resync interval (default: hourly)
- LCD pin defines, if your wiring differs from the table above

## Flashing

1. Open `Watch.ino` in the Arduino IDE (all `.h`/`.cpp` files in the folder are picked up automatically).
2. Select your NodeMCU board and correct COM port.
3. Upload.
4. Open the Serial Monitor at 115200 baud to see the assigned IP address once Wi-Fi connects.

## Using it

Visit `http://<device-ip>/` in a browser on the same network. The page shows live Clock, Timer, and Stopwatch panels and updates every ~300 ms without reloading. Each panel has a **Show on LCD** button that switches what the physical LCD displays — this never pauses or resets the Timer or Stopwatch, it only changes what's currently visible on the small screen.

## HTTP API

| Method | Endpoint | Notes |
|---|---|---|
| GET | `/` | Web UI |
| GET | `/status` | JSON status of clock, timer, stopwatch, and current display mode |
| GET | `/display?mode=clock\|timer\|stopwatch` | Switch LCD view |
| POST | `/timer/start[?hours=&minutes=&seconds=]` | Start (optionally set a new duration first) |
| POST | `/timer/pause` | Pause |
| POST | `/timer/resume` | Resume |
| POST | `/timer/reset` | Reset to configured duration |
| POST | `/stopwatch/start` | Start from zero |
| POST | `/stopwatch/pause` | Pause |
| POST | `/stopwatch/resume` | Resume |
| POST | `/stopwatch/reset` | Reset to zero |

Example `/status` response:

```json
{
  "display": "timer",
  "clock": "14:25:36",
  "timer": "00:10:14",
  "timerRunning": true,
  "stopwatch": "03:25.47",
  "stopwatchRunning": false
}
```

## Architecture notes

- Each module (`clock_manager`, `timer_manager`, `stopwatch_manager`, `lcd_manager`, `webserver_manager`) owns its own state and is driven from `loop()` via a single `update()` call — no cross-module polling.
- `lcd_manager` keeps a shadow copy of both LCD rows and only rewrites characters that actually changed, avoiding visible flicker without ever calling `lcd.clear()` on every frame.
- Timer and Stopwatch derive elapsed/remaining time from `millis()` deltas rather than counting loop iterations, so their accuracy doesn't depend on how often `loop()` runs.
- Wi-Fi credentials and pin mapping are isolated in `config.h`; the web page markup/JS is isolated in `webpage.h`.
