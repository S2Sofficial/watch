# Watch — ESP8266 Wi-Fi Clock, Timer & Stopwatch

Watch turns a NodeMCU (ESP8266) and a 16x2 I2C LCD into a desk clock,
countdown timer, and stopwatch that is controlled entirely from a web
browser on your phone, tablet, or computer. There are no physical
buttons, switches, or a buzzer on the device itself — every action
(setting a timer, starting the stopwatch, switching what the LCD
shows) happens over Wi-Fi from the page the device serves.

<img width="1280" height="720" alt="WATCH Project" src="https://github.com/user-attachments/assets/fad18f48-db20-4f57-9abf-1e1407d88df1" />

## Why a Wi-Fi-controlled watch

Because there is no on-device input hardware to wire up or fail, the
whole build is four wires (power + two I2C lines) to the LCD and
nothing else. That has a couple of practical benefits:

- **Control from anywhere on the network.** Start a countdown from
  across the room, or hand the page to someone else, without touching
  the device.
- **Three tools, one small footprint.** The clock, timer, and
  stopwatch all run continuously and independently in the background;
  the LCD is just a window onto whichever one you currently want to
  see. Switching what's shown on the LCD never pauses or resets the
  other two, so, for example, a countdown keeps running exactly on
  schedule while the LCD is showing the clock.
- **Useful as a real desk tool.** Because control is a page on your
  phone rather than tiny physical buttons, it is genuinely usable for
  things like timing a task or a workout, running a kitchen-style
  countdown, or just keeping an always-accurate, NTP-synced clock on
  a shelf — without needing to touch the hardware at all once it's
  plugged in.

## What's new in version 3

Version 3 is a hardware and reliability upgrade over previous
versions:

- **LCD wiring switched from parallel 4-bit to I2C.** Earlier
  versions drove the 16x2 LCD directly over six data/control wires.
  Version 3 uses an I2C backpack (PCF8574-based) instead, cutting the
  wiring down to two signal wires (SDA/SCL) plus power/ground and
  giving a much more compact final build.
- **The web UI is noticeably more responsive.** Several causes of
  input lag/stalls in the control loop were identified and fixed (see
  [Design notes](#design-notes) for details): a periodic NTP
  operation that used to briefly freeze the whole device roughly once
  a minute, the ESP8266's default Wi-Fi power-saving mode, and TCP-level
  delays on HTTP responses. Button presses and the live status
  display should now feel immediate rather than occasionally sticking
  or lagging.
- **Wi-Fi loss is now visible and recoverable.** If the device loses
  its Wi-Fi connection, it now shows this on the LCD and keeps
  retrying automatically in the background instead of silently going
  unreachable. See [Display messages reference](#display-messages-reference).
- **The IP address is shown on the LCD**, at boot and again after any
  reconnect, so you can find the web UI without a serial monitor.
- **The clock no longer shows a misleading time before its first sync
  completes** — it explicitly shows that it's syncing instead of a
  plausible-looking but wrong time.
- Minor stability and correctness fixes: reduced memory churn on the
  status endpoint (which is polled several times a second by the web
  page), and out-of-range values sent to the timer's start endpoint
  are now clamped instead of silently wrapping to an unrelated value.

## Project layout

```
Watch/
├── Watch.ino               Main sketch: wiring of modules, setup()/loop()
├── config.h                Wi-Fi/NTP settings, LCD I2C address/pins, shared enums
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

| Library            | Author                          | Notes                                    |
|--------------------|----------------------------------|-------------------------------------------|
| NTPClient          | Fabrice Weinberg                 | Time sync                                  |
| LiquidCrystal_I2C  | Frank de Brabander (or compatible fork) | Search "LiquidCrystal I2C" in Library Manager |

`Wire` (I2C), `ESP8266WiFi`, `ESP8266WebServer`, and `WiFiUdp` ship
with the **ESP8266 board package** (install via Boards Manager:
search "esp8266" by ESP8266 Community, if not already installed).

Board selection: **NodeMCU 1.0 (ESP-12E Module)**.

## 2. Wiring (16x2 LCD over I2C)

| LCD/Backpack Pin | NodeMCU Pin | GPIO  |
|-------------------|-------------|-------|
| GND               | GND         | —     |
| VCC               | VIN (5V)    | —     |
| SDA               | D2          | GPIO4 |
| SCL               | D1          | GPIO5 |

That's it — only four wires, down from the six required by the
parallel 4-bit wiring used in earlier versions. Contrast and
backlight are handled by the potentiometer/jumper already on the I2C
backpack, so no external potentiometer or resistor is needed.

If the display stays blank or shows solid black/garbled blocks after
wiring:
- Turn the small potentiometer on the back of the I2C module — this
  sets contrast and is the most common cause of an apparently "dead"
  display.
- Double-check the I2C address. `0x27` and `0x3F` are the two common
  defaults; if unsure, run an I2C scanner sketch and update
  `LCD_I2C_ADDRESS` in `config.h` to match.
- Power VCC from `VIN` (5V, present only when powered via USB), not
  `3V3` — most PCF8574 backpacks and their backlight LED expect 5V.
- Loose or long/noisy SDA/SCL wiring can cause intermittent garbled
  characters. Keep the I2C wires short, and check the connections if
  this happens sporadically rather than every time the device boots.

## 3. Configuration

Edit `config.h` before flashing:

```cpp
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define GMT_OFFSET_SEC  19800   // e.g. 19800 = UTC+5:30 (India)
#define LCD_I2C_ADDRESS 0x27    // change to 0x3F if display stays blank
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
   the assigned IP address. This is optional — see the next section
   for how to find the IP address from the LCD instead.

## 5. Using it

- On boot, the LCD briefly shows the device's IP address (see
  [Display messages reference](#display-messages-reference)) — visit
  `http://<that-ip>/` in a browser on the same network to open the
  control page (Clock / Timer / Stopwatch cards + display selector).
  No serial monitor is required to find it.
- The LCD always shows exactly one module; all three keep running
  in the background regardless of which one is displayed.
- The small dot next to "Display Selector" on the web page indicates
  whether the page is currently able to reach the device (lit = the
  last status poll succeeded, dim = it failed, e.g. the device is
  unreachable or you've lost your own network connection).
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

## Display messages reference

The 16x2 LCD only ever shows one screen at a time. Below is every
message it can show, in priority order — a Wi-Fi problem, for
instance, is always shown even if the Timer is selected, because
without Wi-Fi the web UI can't be reached anyway.

| Line 1       | Line 2              | When it appears                                                                 | Meaning / what to do |
|--------------|---------------------|----------------------------------------------------------------------------------|-----------------------|
| `Wifi Lost!` | `Reconnecting...`   | The device has lost its Wi-Fi connection.                                        | The device is retrying automatically in the background; no action needed. If it persists, check that the AP/router is up and that `WIFI_SSID`/`WIFI_PASSWORD` in `config.h` are still correct. |
| `IP Address` | e.g. `192.168.1.42` | For a few seconds right after boot, and again for a few seconds right after Wi-Fi reconnects. | Use this address to open the web UI: `http://<address>/`. |
| `Clock`      | `Syncing...`        | Clock is selected, but the device has not yet completed its first time sync.     | Wait a few seconds; this clears automatically once the first NTP sync succeeds. If it never clears, check your internet/router connection (NTP requires outbound UDP to `pool.ntp.org`). |
| `Clock`      | `HH:MM:SS`          | Clock is selected and synced.                                                    | Normal 24-hour clock display, resynced hourly. |
| `Timer`      | `HH:MM:SS`          | Timer is selected.                                                               | Time remaining on the countdown, set from the web UI. |
| `Stopwatch`  | `MM:SS.hh`          | Stopwatch is selected.                                                           | Elapsed time (`hh` = hundredths of a second). |

<img width="1280" height="720" alt="LED References" src="https://github.com/user-attachments/assets/ed64e226-b552-4f08-8482-2f889e544d26" />

The web page also shows a status badge on the Timer and Stopwatch
cards:

| Badge      | Meaning |
|------------|---------|
| `idle`     | Not started yet (or reset). |
| `running`  | Actively counting. |
| `paused`   | Stopped midway; resuming continues from here. |
| `finished` | Timer only — the countdown reached zero. |

## 6. HTTP API reference

| Method | Path                       | Purpose                                   |
|--------|----------------------------|--------------------------------------------|
| GET    | `/`                        | Web UI                                    |
| GET    | `/status`                  | Full JSON status                          |
| GET    | `/display?mode=clock`      | Show Clock on LCD                         |
| GET    | `/display?mode=timer`      | Show Timer on LCD                         |
| GET    | `/display?mode=stopwatch`  | Show Stopwatch on LCD                     |
| POST   | `/timer/start`             | Body: `h=<int>&m=<int>&s=<int>` — sets duration and starts (values are clamped to 0-99 hours, 0-59 minutes/seconds) |
| POST   | `/timer/pause`             | Pause timer                               |
| POST   | `/timer/resume`            | Resume a paused timer                     |
| POST   | `/timer/reset`             | Reset to last configured duration         |
| POST   | `/stopwatch/start`         | Start stopwatch from zero                 |
| POST   | `/stopwatch/pause`         | Pause stopwatch                            |
| POST   | `/stopwatch/resume`        | Resume a paused stopwatch                  |
| POST   | `/stopwatch/reset`         | Reset stopwatch to zero                    |

## Design notes

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
- **Why the web UI used to feel choppy, and what changed**: the NTP
  library's internal update call blocks the entire device for up to
  about a second while it waits for a reply, and it used to run once
  a minute — freezing button presses and the live status poll right
  along with it. It's now only allowed to run on the same hourly
  schedule as the explicit resync, since that already satisfies the
  "resync hourly" requirement and the clock's time is extrapolated
  from `millis()` between syncs regardless. On top of that, the
  ESP8266's default Wi-Fi power-saving mode and Nagle's algorithm on
  HTTP responses both add real-world latency to every request; both
  are now disabled, since this device is always mains/USB powered and
  has no need for either.
- **Wi-Fi resilience**: `Watch.ino` checks the Wi-Fi link every loop
  iteration (cheap, non-blocking). On loss it logs to Serial, retries
  in the background, and shows a status message on the LCD; on
  recovery it logs the new IP, briefly shows it on the LCD, and
  triggers an early clock resync in case time drifted while offline.
