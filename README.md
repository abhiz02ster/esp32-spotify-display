# ESP32 Spotify Display

This project utilizes an ESP32 module and a Waveshare 1.54" Black & White e-ink display to showcase the album art of the currently playing song on Spotify in beautiful dithered monochrome.

<img src="readme-images/demo.png"/>

## Overview

The ESP32 module connects to a Python Flask server which processes Spotify API data, applies Floyd-Steinberg dithering to the album art, packs it into a 1-bit monochrome byte stream, and serves it. The ESP32 pulls this stream over WiFi and renders the album art on the e-ink display in real-time.

## Requirements
- ESP32 module with WiFi connectivity. (e.g. [ESP32 WROOM 32](https://www.amazon.ca/ESP-WROOM-32-NodeMCU-Development-Bluetooth-Microcontroller/dp/B0C9VSYS4N)).
- A [Waveshare 1.54" Black & White e-Paper display module](https://www.waveshare.com/1.54inch-e-paper-module.htm) (200x200 resolution).
- Spotify Premium account. You will need this to create a developer account and get API access.
- Visual Studio Code with PlatformIO (Recommended) or Arduino IDE.

## Pin Configuration (E-Ink to ESP32)

Connect your e-ink display to the ESP32 using the following SPI pin configuration:

| E-Ink Pin | ESP32 Pin | Description |
|-----------|-----------|-------------|
| **VCC**   | 3.3V      | Power       |
| **GND**   | GND       | Ground      |
| **DIN**   | GPIO 23   | SPI MOSI    |
| **CLK**   | GPIO 18   | SPI SCK     |
| **CS**    | GPIO 5    | Chip Select |
| **DC**    | GPIO 17   | Data/Command|
| **RST**   | GPIO 16   | Reset       |
| **BUSY**  | GPIO 4    | Busy Pin    |

---

## Setup

1. [Spotify API Tokens](#spotify-api-tokens)
2. [Server Setup](#server-setup)
3. [ESP32 Setup](#esp32-setup)

---

## Spotify API Tokens

1. Go to https://developer.spotify.com/dashboard
2. Press the "Create app" button
3. Give your app a name and a short description
4. Copy and paste the following url into the "Redirect URI" field: `https://alecchen.dev/spotify-refresh-token`
5. Check Web API under "Which API/SDKs are you planning to use?"
6. Check the "I understand" box to accept Spotify's Developer Terms of Service and Design Guidelines
7. Press the "Save" button
8. Now, click the "Settings" button
9. Click "View client secret"
10. Save the "Client ID" and "Client secret" somewhere for now, we will need them for the next step.
11. Open the following website https://alecchen.dev/spotify-refresh-token/
12. Paste in your Client ID and Client secret.
13. Under Scope, select: `user-read-playback-state` and `user-read-currently-playing`
14. Click submit. You will be redirected to Spotify to log in and click "Agree".
15. Copy the generated 'Refresh Token' from the final screen and save it.

---

## Server Setup

1. Clone or download the repository.
2. Open the `server` directory and rename `.env.template` to `.env`.
3. Paste in your Spotify Client ID, Client Secret, and Refresh Token.
4. Run the server using one of the following methods:
    - **Method A: Flask CLI (Local)**
      - Open terminal in the project root directory
      - Install packages using:
        ```bash
        pip install -r server/requirements.txt
        ```
      - Run the server on port 11212:
        ```bash
        flask --app server/server.py run --host=0.0.0.0 --port=11212
        ```
      - Copy your Mac/computer's local IP address (e.g. `http://192.168.0.76:11212`) to configure the ESP32.
    - **Method B: Docker Container**
      - Build and run the container mapping port 11212:
        ```bash
        docker image build -t flask_docker ./server && docker run -p 11212:11212 -d flask_docker
        ```

---

## ESP32 Setup

You can build and flash the firmware using **PlatformIO** or the **Arduino IDE**:

### Method 1: PlatformIO (Recommended)
1. Open the root folder in VS Code with PlatformIO installed.
2. Open `src/env.h` (or copy `embedded/template.env.h` to `src/env.h`).
3. Fill in your `SERVER_IP` (e.g., `"http://192.168.0.76:11212"`), `WIFI_SSID`, and `PASSWORD`.
4. Click the PlatformIO **Upload** button to automatically download libraries (`GxEPD2`, `Adafruit_GFX`, `ArduinoJson`), compile, and flash the ESP32.

### Method 2: Arduino IDE
1. Open the `embedded/` directory.
2. Rename `template.env.h` to `env.h` and fill in your WiFi credentials and server IP.
3. Open `embedded.ino` in the Arduino IDE.
4. Go to **Library Manager** and download the following libraries:
   - `Adafruit_GFX`
   - `GxEPD2`
   - `ArduinoJson`
5. Select your ESP32 board, compile, and upload.
