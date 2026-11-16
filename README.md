# Lynx

Lynx is a smart LED display designed to show real-time information such as the time, weather, air quality, city bike availability, and public transport schedules.

![Lynx demo](lynx.gif)

## Hardware

- **Adafruit Matrix Portal S3** (ESP32-S3–based controller designed to drive RGB LED matrix displays)
- **RGB LED matrix panel (64×32)**
- **3D-printed case**

## Screens

| Name | Content |
| --- | --- |
| `clock` | Current time and date |
| `weather` | Current weather and air quality |
| `forecast` | Weather for the next two days |
| `hourly` | Temperature curve and rain for the next 12 hours |
| `sun` | Sunrise and sunset |
| `bike` | City bike availability at a selected station (Lyon’s Vélo’v) |
| `transport` | Next metro departures; alternates with the scrolling alert text when the line is disrupted |

Screens slide in from the right. A red pixel in the top-right corner means the data is outdated (the last syncs failed).

## APIs

- Weather and air quality: [OpenMeteo](https://open-meteo.com)
- City bike and transport: [data.grandlyon](https://data.grandlyon.com)

## Controls

- **Tap on the case**: next screen
- **Double tap**: pause or resume the rotation (a blue pixel in the top-left corner shows it is paused)
- **Up / Down buttons**: brightness +5% / -5% (hold to repeat, remembered across reboots), shown as a gauge.
  Also adjustable live from the web page
- During night mode, the first action only wakes the display up for a minute
- A tap dismisses a message

A single tap is acted on about 0.4 s after the knock, the time needed to tell it apart from a double tap.

## Web page

`http://lynx.local` (user `admin`, password `admin_password` if set) shows a live preview of the display,
the board's status (version, uptime, Wi-Fi, RAM usage, last sync and error of each data source),
a form to send a message, every setting, and firmware updates.

## Messages

Display a message from any device on the network, e.g. a phone shortcut or Home Assistant:

```sh
curl -u admin:change-me http://lynx.local/message \
  -H 'Content-Type: application/json' \
  -d '{"text": "Laundry is done", "duration": 20, "color": "#ff8800"}'

curl -u admin:change-me http://lynx.local/message -d 'text=Hello'   # Form fields work too
```

`duration` is in seconds (default 15, max 600) and `color` defaults to white.
Messages wake the display up during night mode.

## Brightness

```sh
curl -u admin:change-me http://lynx.local/brightness -d 'brightness=0.4'   # Or JSON {"brightness": 0.4}
```

From `0.05` to `1.0`, rounded to 5% steps and remembered across reboots. During night mode, it wakes the display up for a minute.

## Configuration

The configuration lives in `config.json` on the board's flash. There are two ways to set it:

- **Web page** (recommended): browse to `http://lynx.local` (user `admin`, password `admin_password`).
  It offers a form for every option, screen ordering, a raw JSON editor and firmware updates.
  On first boot, or if Wi-Fi can't connect within 30 s, Lynx opens a `Lynx-Setup` Wi-Fi network:
  join it and the same page opens (otherwise browse to `http://192.168.4.1`).
  Saving restarts the board. Passwords left empty are kept unchanged.
- **File**: create `data/config.json` and upload it with `pio run -t uploadfs`.

| Key | Type | Description |
| --- | --- | --- |
| `wifi_ssid` | string | **Required.** Wi-Fi network SSID |
| `wifi_password` | string | Wi-Fi password |
| `admin_password` | string | Protects the web page (user `admin`) and OTA updates. Strongly recommended |
| `brightness` | float | Initial brightness, `0.05` to `1.0` (default `0.5`), until changed with the buttons or the web page |
| `screen_interval` | integer | Default time per screen in ms (default `10000`) |
| `screens` | array | Screen order, using the names above. Each entry is a name or `{"name": ..., "duration": ms}`. Defaults to all configured screens |
| `tap_threshold` | integer | Tap detection threshold, `1` (most sensitive) to `127`, `0` disables it (default `60`) |
| `latitude`, `longitude` | float | Location for weather and air quality. Weather screens are disabled without them |
| `timezone` | string | [POSIX TZ string](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv), DST included (default `CET-1CEST,M3.5.0,M10.5.0/3`, Europe/Paris) |
| `night_start`, `night_end` | string | Night mode window, `"HH:MM"`. Disabled if missing |
| `night_brightness` | float | Brightness during night mode, `0` turns the display off (default `0`) |
| `api_bike_station_id` | integer | Vélo'v station ID. Bike screen disabled without it |
| `api_transport_stop_id_1` | integer | First stop ID. Transport screen disabled without it |
| `api_transport_stop_id_2` | integer | Optional second stop ID (e.g. the other direction) |
| `api_transport_ligne_cli` | string | Line name (e.g. `B`), used for alerts and the line badge |
| `api_transport_username`, `api_transport_password` | string | data.grandlyon credentials |

### Example `config.json`

```json
{
  "wifi_ssid": "my-wifi",
  "wifi_password": "password",
  "admin_password": "change-me",
  "brightness": 0.8,
  "screen_interval": 10000,
  "screens": ["clock", "weather", "forecast", "bike", {"name": "transport", "duration": 15000}],
  "latitude": 45.7640,
  "longitude": 4.8357,
  "timezone": "CET-1CEST,M3.5.0,M10.5.0/3",
  "night_start": "23:00",
  "night_end": "07:00",
  "night_brightness": 0,
  "api_bike_station_id": 10022,
  "api_transport_stop_id_1": 42732,
  "api_transport_stop_id_2": 46054,
  "api_transport_ligne_cli": "B",
  "api_transport_username": "username",
  "api_transport_password": "password"
}
```

## Build and update

The first flash of this firmware must be done over USB. After that, updates can go over Wi-Fi:

```sh
pio run -t upload                                        # USB
LYNX_OTA_PASSWORD=change-me pio run -e ota -t upload     # Wi-Fi, from PlatformIO
pio run                                                  # Or build, then install
                                                         # .pio/build/adafruit_matrixportal_esp32s3/firmware.bin
                                                         # from the "Firmware update" section of the web page
pio device monitor                                       # Logs (USB)
```

## Development

```sh
pio test -e native                 # Unit tests of the hardware-independent logic (src/core), run on the computer
python3 scripts/bitmaps.py --trim  # Regenerate src/bitmap.h/.cpp after editing images/*.bmp (needs Pillow)
```

Images are cropped to their content (black is transparent); layouts center them in their slot when needed.

GitHub Actions runs the tests and builds the firmware on every push and pull request.
Pushing a `v*` tag (e.g. `git tag v1.0.0 && git push --tags`) publishes a release with `firmware.bin`,
ready to install from the web page. The version shown on the web page comes from `git describe`.
