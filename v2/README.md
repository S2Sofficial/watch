# Watch — ESP8266 Wi-Fi Clock, Timer & Stopwatch

A modular, non-blocking ESP8266 (NodeMCU) project driving a 16x2
parallel LCD, with a responsive web UI for control and an NTP-backed
clock. No RTC, buttons, or buzzer required.

## Project layout

```
Watch/
├── Watch.ino               Main sketch: wiring of modules, setup()/loop()
├── config.h                Wi-Fi/NTP settings, LCD pins, shared enums
├── webpage.h                Single-page web UI (HTML/CSS/JS in PROGMEM)
├── lcd_manager.h/.cpp       Flicker-minimized partial LCD updates
├── clock_manager.h/.cpp     NTP sync + time formatting
├── timer_manager.h/.cpp     Countdown timer state machine
├── stopwatch_manager.h/.cpp Stopwatch state machine (millis-based)
└── webserver_manager.h/.cpp HTTP routes + JSON status endpoint
```

## 1. Required libraries

Install via the Arduino Library Manager (Sketch → Include Library →
Manage Libraries):

| Library        | Author            | Notes                                |
|----------------|--------------------|---------------------------------------|
| NTPClient      | Fabrice Weinberg   | Only third-party dependency needed    |
| LiquidCrystal  | Arduino (built-in) | Ships with the Arduino IDE            |

`ESP8266WiFi`, `ESP8266WebServer`, and `WiFiUdp` ship with the
**ESP8266 board package** (install via Boards Manager: search
"esp8266" by ESP8266 Community, if not already installed).

Board selection: **NodeMCU 1.0 (ESP-12E Module)**.

## 2. Wiring (16x2 LCD, 4-bit parallel mode)

| LCD Pin | NodeMCU Pin | GPIO   |
|---------|-------------|--------|
| RS      | D1          | GPIO5  |
| E       | D2          | GPIO4  |
| D4      | D5          | GPIO14 |
| D5      | D6          | GPIO12 |
| D6      | D7          | GPIO13 |
| D7      | D0          | GPIO16 |
| RW      | GND         | —      |
| VSS     | GND         | —      |
| VDD     | 5V (or 3V3 depending on LCD) | — |
| V0      | Wiper of a 10k potentiometer between VDD/GND (contrast) | — |
| A / K   | Backlight +5V / GND (through resistor if not built-in) | — |

Pins were chosen specifically to avoid the ESP8266 boot-strapping
pins (D3/GPIO0, D4/GPIO2, D8/GPIO15), so the board boots reliably
regardless of how the LCD lines are sitting at power-up.

## 3. Configuration

Edit `config.h` before flashing:

```cpp
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define GMT_OFFSET_SEC  19800   // e.g. 19800 = UTC+5:30 (India)
```

`GMT_OFFSET_SEC` is currently set to `19800` (India Standard Time).
Adjust for your timezone (seconds offset from UTC, negative for
timezones west of UTC).

## 4. Flashing

1. Open `Watch.ino` in the Arduino IDE (all other files in the folder
   are picked up automatically as part of the sketch).
2. Select board **NodeMCU 1.0 (ESP-12E Module)** and the correct COM
   port.
3. Upload.
4. Open the Serial Monitor at 115200 baud to see Wi-Fi/NTP status and
   the assigned IP address.

## 5. Using it

- Visit `http://<device-ip>/` in a browser on the same network for
  the control page (Clock / Timer / Stopwatch cards + display
  selector).
- The LCD always shows exactly one module; all three keep running
  in the background regardless of which one is displayed.
- `GET /status` returns live JSON state for all three modules, e.g.:

  ```json
  {
    "display": "timer",
    "clock": "14:25:36",
    "timer": "00:10:14",
    "timerRunning": true,
    "timerState": "running",
    "stopwatch": "03:25.47",
    "stopwatchRunning": false,
    "stopwatchState": "idle"
  }
  ```

## 6. HTTP API reference

| Method | Path                       | Purpose                                   |
|--------|----------------------------|--------------------------------------------|
| GET    | `/`                        | Web UI                                    |
| GET    | `/status`                  | Full JSON status                          |
| GET    | `/display?mode=clock`      | Show Clock on LCD                         |
| GET    | `/display?mode=timer`      | Show Timer on LCD                         |
| GET    | `/display?mode=stopwatch`  | Show Stopwatch on LCD                     |
| POST   | `/timer/start`             | Body: `h=<int>&m=<int>&s=<int>` — sets duration and starts |
| POST   | `/timer/pause`             | Pause timer                               |
| POST   | `/timer/resume`            | Resume a paused timer                     |
| POST   | `/timer/reset`             | Reset to last configured duration         |
| POST   | `/stopwatch/start`         | Start stopwatch from zero                 |
| POST   | `/stopwatch/pause`         | Pause stopwatch                            |
| POST   | `/stopwatch/resume`        | Resume a paused stopwatch                  |
| POST   | `/stopwatch/reset`         | Reset stopwatch to zero                    |

## 7. Design notes

- **Non-blocking loop**: `loop()` only calls `handleWebServer()`,
  `updateClock()`, `updateTimer()`, `updateStopwatch()`, and
  `updateLCD()` — no `delay()` anywhere in the runtime path. The only
  blocking calls are the one-time Wi-Fi connect and initial NTP sync
  inside `setup()`.
- **Flicker-free LCD**: `LCDManager` never calls `lcd.clear()` after
  boot; it diffs each new line against what is already on screen and
  rewrites only the characters that changed, throttled to
  `LCD_REFRESH_INTERVAL_MS`.
- **Drift-resistant timer**: the countdown advances by whole seconds
  consumed since the last tick (using `elapsed / 1000`), rather than
  resetting its reference every loop iteration, so it won't slowly
  drift under variable loop timing.
- **Independent modules**: `currentDisplay` (in `Watch.ino`) is the
  only thing the web UI's display selector touches. The Timer and
  Stopwatch classes have no awareness of what is on the LCD.
