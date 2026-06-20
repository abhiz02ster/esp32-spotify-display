#include <Arduino.h>
#include "env.h"

#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <WiFi.h>

// screen size definitions
#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 200

// display pin configuration
#define EPD_CS    5
#define EPD_DC    17
#define EPD_RST   16
#define EPD_BUSY  4

// Instantiate e-ink display for Waveshare 1.54" 200x200 B/W
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// how frequently to make a call to the server (in miliseconds)
unsigned long timerDelay = 10000;
unsigned long lastTime = 0;

// will store the 1-bit packed monochrome image data
uint8_t albumCover[5000];

void setup()
{
    Serial.begin(115200);

    // connect to wifi
    WiFi.persistent(false);          // do not save credentials to flash
    WiFi.mode(WIFI_STA);
    // disconnect(wifioff=false, eraseap=true): keep radio ON, erase stale saved AP
    WiFi.disconnect(false, true);
    delay(500);                       // let radio settle before begin

    WiFi.begin(WIFI_SSID, PASSWORD);
    Serial.print("Connecting to WiFi");

    int wifiRetries = 0;
    const int maxRetries = 40; // 40 x 500ms = 20 seconds timeout
    while (WiFi.status() != WL_CONNECTED && wifiRetries < maxRetries)
    {
        Serial.print(".");
        // print status code every 10 dots so we can see what's happening
        if (wifiRetries > 0 && wifiRetries % 10 == 0)
        {
            Serial.print(" [status=");
            Serial.print(WiFi.status());
            Serial.print("] ");
        }
        delay(500);
        wifiRetries++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("");
        Serial.print("Failed to connect to WiFi. Final status code: ");
        Serial.println(WiFi.status());
        // Status codes: 0=IDLE, 1=NO_SSID_AVAIL, 3=CONNECTED,
        //               4=CONNECT_FAILED (wrong password), 6=DISCONNECTED, 255=NO_SHIELD
        Serial.print("SSID attempted: "); Serial.println(WIFI_SSID);
        Serial.println("Halting. Fix credentials/network and reflash.");
        while (true) { delay(1000); }
    }

    Serial.println("");
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");

    // initialize display
    display.init(115200);
    Serial.println("Initialized E-Ink display");

    // initially clear display
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
    }
    while (display.nextPage());
}

void loop()
{
    if ((millis() - lastTime) > timerDelay)
    {
        // check if we're connected to the internet
        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;

            // Send request
            http.useHTTP10(true);
            http.begin(client, SERVER_IP);
            int httpCode = http.GET();

            if (httpCode > 0)
            {
                // create a stream
                WiFiClient *stream = http.getStreamPtr();

                if (stream != nullptr)
                {
                    bool hasSong = true;
                    bool inArray = false;
                    String buff = "";
                    int n = 0;
                    String headerSearch = "";

                    while (stream->connected() || stream->available() > 0)
                    {
                        if (stream->available() > 0)
                        {
                            uint8_t readBuf[256];
                            int len = stream->read(readBuf, sizeof(readBuf));
                            for (int i = 0; i < len; i++)
                            {
                                char c = (char)readBuf[i];

                                if (!inArray)
                                {
                                    headerSearch += c;
                                    if (headerSearch.length() > 40)
                                    {
                                        headerSearch.remove(0, 1);
                                    }

                                    if (headerSearch.indexOf("no-song") != -1)
                                    {
                                        hasSong = false;
                                        break;
                                    }

                                    if (c == '[')
                                    {
                                        inArray = true;
                                    }
                                }
                                else
                                {
                                    if (c == ',')
                                    {
                                        if (n < 5000)
                                        {
                                            albumCover[n] = (uint8_t)buff.toInt();
                                            n++;
                                        }
                                        buff = "";
                                    }
                                    else if (c == ']')
                                    {
                                        if (buff.length() > 0 && n < 5000)
                                        {
                                            albumCover[n] = (uint8_t)buff.toInt();
                                            n++;
                                        }
                                        inArray = false;
                                        break;
                                    }
                                    else if (c >= '0' && c <= '9')
                                    {
                                        buff += c;
                                    }
                                }
                            }

                            if (!hasSong || (!inArray && n > 0))
                            {
                                break;
                            }
                        }
                        else
                        {
                            delay(1);
                        }
                    }

                    if (hasSong && n > 0)
                    {
                        Serial.print("Successfully read ");
                        Serial.print(n);
                        Serial.println(" bytes.");
                    }
                }
                else
                {
                    Serial.println("Error: Stream pointer is null");
                }
            }
            else
            {
                Serial.print("HTTP GET failed, error: ");
                Serial.println(http.errorToString(httpCode));
            }

            // Disconnect
            http.end();
        }
        else
        {
            Serial.println("Not connected to WiFi");
        }

        lastTime = millis();

        Serial.println("Updating display...");
        display.firstPage();
        do
        {
            display.fillScreen(GxEPD_WHITE);
            display.drawBitmap(0, 0, albumCover, SCREEN_WIDTH, SCREEN_HEIGHT, GxEPD_BLACK);
        }
        while (display.nextPage());
    }
}