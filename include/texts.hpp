#pragma once

#include <TFT_eSPI.h>
#include <time.h>
#include <include/bedridden.hpp>

// weather
TFTPrint textTime;
TFTPrint textDate;
TFTPrint textWeek;
TFTPrint textTemperature;
TFTPrint textHumidity;
TFTPrint textAlarm;

TFTPrint textTodayTemperature;
TFTPrint textNow;
TFTPrint textWind;

TFTPrint text6AM;
TFTPrint text12PM;
TFTPrint text6PM;
TFTPrint text12AM;
TFTPrint textTemperature6AM;
TFTPrint textTemperature12PM;
TFTPrint textTemperature6PM;
TFTPrint textTemperature12AM;

// settings
TFTPrint textSettings;
TFTPrint textLighting;
TFTPrint textRefresh;
TFTPrint textReboot;


void initTexts()
{
    textTime.begin(85, 239, 30, 5, TFT_WHITE, TFT_BLACK);
    textDate.begin(80, 239, 99, 2, TFT_WHITE, TFT_BLACK);
    textWeek.begin(80, 239, 119, 2, TFT_WHITE, TFT_BLACK);

    textTemperature.begin(270, 93, 2, TFT_WHITE, TFT_BLACK);
    textHumidity.begin(270, 133, 2, TFT_WHITE, TFT_BLACK);
    textAlarm.begin(270, 53, 2, TFT_WHITE, TFT_BLACK);

    textNow.begin(0, 79, 10, 1, TFT_WHITE, TFT_BLACK);
    textTodayTemperature.begin(0, 79, 10 + 20, 2, TFT_WHITE, TFT_BLACK);
    textWind.begin(0, 79, 30 + 20, 2, TFT_WHITE, TFT_BLACK);

    text6AM.begin(0, 79, 159 + 4, 1, TFT_WHITE, TFT_BLACK);
    text12PM.begin(80, 159, 159 + 4, 1, TFT_WHITE, TFT_BLACK);
    text6PM.begin(160, 239, 159 + 4, 1, TFT_WHITE, TFT_BLACK);
    text12AM.begin(240, 319, 159 + 4, 1, TFT_WHITE, TFT_BLACK);
    textTemperature6AM.begin(0, 79, 159 + 15, 2, TFT_WHITE, TFT_BLACK);
    textTemperature6PM.begin(160, 239, 159 + 15, 2, TFT_WHITE, TFT_BLACK);
    textTemperature12PM.begin(80, 159, 159 + 15, 2, TFT_WHITE, TFT_BLACK);
    textTemperature12AM.begin(240, 319, 159 + 15, 2, TFT_WHITE, TFT_BLACK);

    // settings
    textSettings.begin(79, 239, 7, 3, TFT_WHITE, TFT_BLACK);
    textLighting.begin(79, 239, 49, 2, TFT_LIGHTGREY, TFT_BLACK);
    textRefresh.begin(79, 239, 130, 2, TFT_LIGHTGREY, TFT_BLACK);
    textReboot.begin(79, 239, 196, 2, TFT_LIGHTGREY, TFT_BLACK);
}

void printStaticTexts()
{
    text6AM.PrintCentered("MORNING");
    text12PM.PrintCentered("NOON");
    text6PM.PrintCentered("EVENING");
    text12AM.PrintCentered("DUSK");
    textAlarm.Print("6:30");
    textNow.PrintCentered("NOW");
}

int lastDay = -1;
void printTimeTexts(bool force = false)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return;

    if (timeinfo.tm_mday != lastDay || force)
    {
        char day[11];
        char date[12];
        char time[6];

        strftime(day, sizeof(day), "%A", &timeinfo);
        snprintf(date, sizeof(date), "%d.%02d.%d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        snprintf(time, sizeof(time), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        textDate.PrintCentered(date);
        textWeek.PrintCentered(day);
        textTime.PrintCentered(time);

        lastDay = timeinfo.tm_mday;
    }
    else
    {
        char time[6];
        snprintf(time, sizeof(time), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        textTime.PrintCentered(time);
    }
}

void printMainWeather(const JsonDocument &doc)
{
    char t6AM[10], t12PM[10], t6PM[10], t12AM[10];

    snprintf(t6AM, sizeof(t6AM), "%.1f\xF7", static_cast<float>(doc["hourly"]["temperature_2m"][6]));
    snprintf(t12PM, sizeof(t12PM), "%.1f\xF7", static_cast<float>(doc["hourly"]["temperature_2m"][12]));
    snprintf(t6PM, sizeof(t6PM), "%.1f\xF7", static_cast<float>(doc["hourly"]["temperature_2m"][18]));
    snprintf(t12AM, sizeof(t12AM), "%.1f\xF7", static_cast<float>(doc["hourly"]["temperature_2m"][24]));

    textTemperature6AM.PrintCentered(t6AM);
    drawIconWeather(static_cast<int>(doc["hourly"]["weather_code"][6]), 14, 194);

    textTemperature12PM.PrintCentered(t12PM);
    drawIconWeather(static_cast<int>(doc["hourly"]["weather_code"][12]), 14 + 80, 194);

    textTemperature6PM.PrintCentered(t6PM);
    drawIconWeather(static_cast<int>(doc["hourly"]["weather_code"][18]), 14 + 80 * 2, 194);

    textTemperature12AM.PrintCentered(t12AM);
    drawIconWeather(static_cast<int>(doc["hourly"]["weather_code"][24]), 14 + 80 * 3, 194);
}

void printCurrentWeather(const JsonDocument &doc) 
{
    char todayTemp[10];
    char wind[7];

    snprintf(todayTemp, sizeof(todayTemp), "%.1f\xF7", static_cast<float>(doc["current"]["temperature_2m"]));
    snprintf(wind, sizeof(wind), "%dkmh", static_cast<int>(doc["current"]["wind_speed_10m"]));

    textTodayTemperature.PrintCentered(todayTemp);
    textWind.PrintCentered(wind);

    drawIconWind(static_cast<int>(doc["current"]["wind_direction_10m"]), 25, 70);
    drawIconWeather(static_cast<int>(doc["current"]["weather_code"]), 0, 97, false, false, 1.55);

}

void printHouseholdSensors()
{
    sensors.fetchAirSensor();
    int array[2];
    sensors.getAirSensor(array);
    textTemperature.Print(String(array[0]) + "\xF7");
    textHumidity.Print(String(array[1]) + "%");
}

void printIcons()
{
    drawBmp("/icons/temperature.bmp", 245, 90);
    drawBmp("/icons/humidity.bmp", 245, 130);
    drawBmp("/icons/settings.bmp", 295, 10);
    drawBmp("/icons/wifi.bmp", 245, 10);
    drawBmp("/icons/clock.bmp", 245, 50);
}

void printSettingsTexts() 
{
    textSettings.PrintCentered("Settings");
    textLighting.PrintCentered("Change Lightning");
    textRefresh.PrintCentered("Refresh sensors");
    textReboot.PrintCentered("Reboot");
}

void printAlarmTexts() 
{
    
}