#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Protomatter.h>
#include <Adafruit_GFX.h>
#include "managers/TimeManager.h"
#include "managers/WeatherManager.h"
#include "managers/ConfigManager.h"
#include "screens/Screen.h"
#include "screens/ClockScreen.h"
#include "screens/CurrentWeatherScreen.h"
#include "screens/ForecastWeatherScreen.h"
#include "screens/BikeScreen.h"
#include "screens/TransportScreen.h"
#include "managers/DisplayManager.h"

// Matrix Portal S3 default pin mapping:
uint8_t rgbPins[] = {42, 40, 41, 38, 37, 39};
uint8_t addrPins[] = {45, 36, 48, 35};
uint8_t clockPin = 2, latchPin = 47, oePin = 14;

Adafruit_Protomatter matrix(
    64, 4, 1, rgbPins, 4, addrPins,
    clockPin, latchPin, oePin, true);

TimeManager timeManager;
WeatherManager weatherManager;
BikeManager bikeManager;
TransportManager transportManager;
ClockScreen clockScreen(&matrix, &timeManager);
CurrentWeatherScreen currentWeatherScreen(&matrix, &weatherManager);
ForecastWeatherScreen forecastWeatherScreen(&matrix, &weatherManager, &timeManager);
BikeScreen bikeScreen(&matrix, &bikeManager);
TransportScreen transportScreen(&matrix, &transportManager);
ConfigManager &config = ConfigManager::getInstance();

unsigned long lastSwitch;
int currentScreen = 0;
Screen *screens[] = {&clockScreen, &currentWeatherScreen, &forecastWeatherScreen, &bikeScreen, &transportScreen};
const int numScreens = sizeof(screens) / sizeof(screens[0]);

void displayLoadingScreen()
{
  const char *text = "Lynx";
  // Text
  int16_t x1, y1;
  uint16_t w, h;
  matrix.setTextSize(1);
  matrix.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // Center coordinates
  int16_t x = (matrix.width() - w) / 2;
  int16_t y = (matrix.height() - h) / 2;

  matrix.fillScreen(0);
  matrix.setCursor(x, y);
  matrix.setTextColor(displayManager.color565(0, 255, 64));
  matrix.print("L");
  matrix.setTextColor(displayManager.color565(255, 128, 0));
  matrix.print("y");
  matrix.setTextColor(displayManager.color565(0, 64, 255));
  matrix.print("n");
  matrix.setTextColor(displayManager.color565(255, 0, 255));
  matrix.print("x");
  matrix.show();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Hello ! Lynx is starting...");

  // Config loading
  if (!config.load())
  {
    Serial.println("Config load failed!");
    while (1)
      ;
  }

  // Matrix init
  ProtomatterStatus status = matrix.begin();
  if (status != PROTOMATTER_OK)
  {
    Serial.printf("Matrix init failed! %d\n", status);
    while (1)
      ;
  }
  matrix.setTextWrap(false);
  displayManager.begin(&matrix);

  displayLoadingScreen();

  // Wifi connection
  Serial.println("Connecting to WiFi...");
  WiFi.begin(config.wifiSsid, config.wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi connected!");

  timeManager.begin();
  weatherManager.begin();
  bikeManager.begin();
  transportManager.begin();

  lastSwitch = millis(); // Start switching screens after the setup
}

void loop()
{
  unsigned long now = millis();

  screens[currentScreen]->update();

  if (now - lastSwitch >= config.screenInterval)
  {
    lastSwitch = now;
    currentScreen = (currentScreen + 1) % numScreens;

    matrix.fillScreen(0);
    matrix.show();
    screens[currentScreen]->draw(); // Draw first time
  }
  delay(100);
}