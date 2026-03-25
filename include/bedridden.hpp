#pragma once

#include <array>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <include/tftprint.hpp>

#define PIN_PHOTORES 34
#define PIN_PROXSENSOR 27
#define PIN_MOTOR 22
#define PIN_AIRSENSOR 21
#define PIN_BUTTON 14
#define SD_PIN 26

class Sensors;
class DHT11;
struct Button {
  char* name;
  int32_t x = 0, y = 0, w = 0, h = 0;
};

extern JsonDocument mainWeather;
extern JsonDocument currentWeather;
extern const char *mainWeatherApi;
extern const char *currentWeatherApi;
extern void printCurrentWeather(const JsonDocument& doc);
extern void printMainWeather(const JsonDocument& doc);

extern const char *ssid;
extern const char *password;
extern HTTPClient http;
extern Sensors sensors;
extern DHT11 dht11;
extern TFT_eSPI tft;
extern bool menuNotChanged;
extern bool forceSensors;

extern void handleInput(std::array<std::array<Button, 5>, 3>& buttons, uint8_t& menu, bool isPhysicallyPressed);

extern void testPos(uint16_t _x, uint16_t _y);
extern void drawLineH(uint16_t _x, uint16_t _y, uint16_t _v, uint16_t color = TFT_LIGHTGREY);
extern void drawLineV(uint16_t _x, uint16_t _y, uint16_t _h, uint16_t color = TFT_LIGHTGREY);
extern void drawBmp(const char *filename, int16_t x, int16_t y, float scale = 1);
extern void drawIconWind(int rotation, int16_t x, int16_t y);
extern void drawIconWeather(int weatherCode, int16_t x, int16_t y, bool wind = false, bool night = false, float scale = 1);
extern void reprintAfterFocus(uint8_t& menu, int8_t& focus);

extern void showSettings();
extern void showAlarm();
extern void showWeather();


extern void printStaticTexts();
extern void printIcons();
extern void printSettingsTexts();
extern void printAlarmTexts();
extern void printTimeTexts(bool force);
