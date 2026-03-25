#include <TFT_eSPI.h>
#include <SPI.h>
#include <DHT11.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SD.h>

#include <include/sensors.hpp>
#include <include/tftprint.hpp>
#include <include/texts.hpp>
#include <include/bedridden.hpp>

const char *ssid = "Orange_Swiatlowod_D610";
const char *password = "DOMEK098";
const char *mainWeatherApi = "https://api.open-meteo.com/v1/forecast?timezone=Europe/Warsaw&latitude=52.42427016836926&longitude=20.969883604650796&models=ecmwf_ifs&hourly=temperature_2m,relative_humidity_2m,weather_code&forecast_days=3";
const char *currentWeatherApi = "https://api.open-meteo.com/v1/forecast?timezone=Europe/Warsaw&latitude=52.42427016836926&longitude=20.969883604650796&models=ecmwf_ifs&current=temperature_2m,weather_code,is_day,wind_direction_10m,wind_speed_10m&forecast_days=3&%3Ftimezone=Europe%2FWarsaw";

HTTPClient http;
TFT_eSPI tft = TFT_eSPI();
DHT11 dht11(PIN_AIRSENSOR);
Sensors sensors;
JsonDocument mainWeather;
JsonDocument currentWeather;

bool wifiStatus;
const char* ntpServer = "pool.ntp.org";
const int64_t gmtOffset_sec = 3600;
const int32_t daylightOffset_sec = 3600;
int64_t lastMinute = -1;
time_t lastSeenSec = 0;

// timers
const uint64_t WEATHER_INTERVAL = 1000 * 60 * 60; // 1 hour
const uint64_t SENSOR_INTERVAL = 1000 * 5;        // 5 seconds
const uint64_t CURRENT_INTERVAL = 1000 * 60 * 15; // 15 min
uint64_t lastSensorUpdate = -SENSOR_INTERVAL;
uint64_t lastWeatherUpdate = -WEATHER_INTERVAL;
uint64_t lastCurrentUpdate = -CURRENT_INTERVAL;
uint64_t lastTimeTime = 0;

// session variables
bool firstBoot = true;
bool menuNotChanged = true;
bool forceSensors = false;
uint8_t menu = 0;
std::array<std::array<Button, 5>, 3> buttons = {{  

  {{
    {"settings", 280, 0, 40, 39}, 
    {"alarm", 240, 40, 79, 39},
    {}, {}, {}
  }},
  {{
    {"home", 280, 0, 40, 39}, 
    {"light", 63, 40, (int32_t)(TFT_HEIGHT * 3 / 5), 65}, 
    {"refresh", 63, 106, (int32_t)(TFT_HEIGHT * 3 / 5), 65}, 
    {"reboot", 63, 172, (int32_t)(TFT_HEIGHT * 3 / 5), 65},
    {}
  }},
  {{
    {"home", 240, 0, 20, 20}, {}, {}, {}, {}
  }}
}};

void setup()
{
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI); 

  tft.init();
  tft.setRotation(3); // 
  tft.fillScreen(TFT_BLACK);
  delay(500); 

  WiFi.mode(WIFI_STA); // wifi setup
  WiFi.disconnect(true);
  delay(100);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 20) { // wifi retry mechanism
    delay(500);
    retryCount++;
    if(retryCount % 5 == 0){
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }
  Serial.println("Wifi connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // time setup

  if(!SD.begin(SD_PIN)) Serial.println("SD Mount Failed"); // fix for retries
  else Serial.println("SD Mounted Successfully");

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  digitalWrite(TFT_BL, HIGH);
  initTexts();

  showWeather();
}

void loop() // alarm - setup, status, detailed weather????
{
  time_t now = time(nullptr); 

  static uint32_t lastTouchTick = 0;
  bool reading = digitalRead(PIN_BUTTON) == LOW;
  handleInput(buttons, menu, reading);

  if (now != lastSeenSec && menu == 0) { // weather actions 
    lastSeenSec = now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      if ( timeinfo.tm_min != lastMinute || forceSensors) { // if a minute passed update the clock
        printTimeTexts();
        lastMinute = timeinfo.tm_min;
      }
      if (( timeinfo.tm_min % CURRENT_INTERVAL == 0 || firstBoot ) && timeinfo.tm_hour != lastWeatherUpdate || forceSensors) { // every 15 minutes checks for current weather
        lastWeatherUpdate = timeinfo.tm_hour;
        if(sensors.fetchWeather(currentWeather, currentWeatherApi)){
          printCurrentWeather(currentWeather);
        }
      }
      if (( timeinfo.tm_min % SENSOR_INTERVAL == 0 || firstBoot ) && timeinfo.tm_min != lastSensorUpdate || forceSensors) { // every 2 minutes checks and updates the weather sensors
        lastWeatherUpdate = timeinfo.tm_min;
        printHouseholdSensors();
      }
      if (( timeinfo.tm_hour % WEATHER_INTERVAL == 0 || firstBoot) && timeinfo.tm_hour != lastCurrentUpdate || forceSensors) { // every 1 hour checks for next hour's weather
        lastCurrentUpdate = timeinfo.tm_hour;
        if(sensors.fetchWeather(mainWeather, mainWeatherApi)){
          printMainWeather(mainWeather); 
        }
      }
    }
    if (forceSensors) forceSensors = false;
  }
  
  // wifi icon change
  if (WiFi.status() == WL_CONNECTED && wifiStatus == false) { wifiStatus = true; drawBmp("/icons/wifi.bmp", 245, 10); }
  else if(WiFi.status() != WL_CONNECTED && wifiStatus == true) { wifiStatus = false; drawBmp("/icons/nowifi.bmp", 245, 10); }

  if (firstBoot) firstBoot = false;
  if (!menuNotChanged) menuNotChanged = true;
  delay(200);
}