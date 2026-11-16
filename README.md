# Lynx

Lynx is a smart LED display designed to show real-time information such as the time, weather, air quality, city bike availability, and public transport schedules.

![Lynx demo](lynx.gif)

## Hardware

- **Adafruit Matrix Portal S3** (ESP32-S3–based controller designed to drive RGB LED matrix displays)
- **RGB LED matrix panel (64×32)**
- **3D-printed case**

## Screens

- Current time and date
- Current weather and air quality
- Weather recap for today and tomorrow
- City bike availability at a selected station (Lyon’s Vélo’v)
- Next metro arrival times and service alerts (Lyon)

## APIs

- Weather and air quality: [OpenMeteo](https://open-meteo.com)
- City bike and transport: [data.grandlyon](https://data.grandlyon.com)

## Buttons

- **Up**: cycle brightness (5% steps, remembered across reboots)
- **Down**: next screen
- During night mode, the first press only wakes the display up for a minute

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
| `brightness` | float | Initial brightness, `0.05` to `1.0` (default `0.5`), until changed with the button |
| `screen_interval` | integer | Default time per screen in ms (default `10000`) |
| `screens` | array | Screen order: `clock`, `weather`, `forecast`, `bike`, `transport`. Each entry is a name or `{"name": ..., "duration": ms}`. Defaults to all configured screens |
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

A red pixel in the top-right corner means the data on screen is outdated (the last syncs failed).
