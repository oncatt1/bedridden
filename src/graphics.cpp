#include <TFT_eSPI.h>
#include <FS.h>
#include <SD.h>

#include <include/bedridden.hpp>

void testPos(uint16_t _x, uint16_t _y)
{
    tft.setCursor(_x, _y);
    tft.drawPixel(_x, _y, TFT_WHITE);
}
void drawLineH(uint16_t _x, uint16_t _y, uint16_t _v, uint16_t color)
{
    tft.setCursor(_x, _y);
    tft.drawFastHLine(_x, _y, _v, color);
}
void drawLineV(uint16_t _x, uint16_t _y, uint16_t _h, uint16_t color)
{
    tft.setCursor(_x, _y);
    tft.drawFastVLine(_x, _y, _h, color);
}

uint16_t read16(fs::File &f)
{
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read();
    ((uint8_t *)&result)[1] = f.read();
    return result;
}

uint32_t read32(fs::File &f)
{
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read();
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read();
    return result;
}

void drawBmp(const char *filename, int16_t x, int16_t y, float scale)
{
    if ((x >= tft.width()) || (y >= tft.height()) || scale <= 0)
        return;

    fs::File bmpFS = SD.open(filename, FILE_READ);
    if (!bmpFS)
    {
        Serial.printf("File not found: %s\n", filename);
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t r, g, b;

    if (read16(bmpFS) == 0x4D42)
    {
        read32(bmpFS);
        read32(bmpFS);
        seekOffset = read32(bmpFS);
        read32(bmpFS);
        w = read32(bmpFS);
        h = read32(bmpFS);

        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
        {
            uint16_t sw = (uint16_t)(w * scale);
            uint16_t sh = (uint16_t)(h * scale);

            // sanity check to avoid huge allocations
            if (sw == 0 || sh == 0 || sw > tft.width() || sh > tft.height())
            {
                Serial.printf("BMP dimensions out of bounds %ux%u (scaled %ux%u)\n", w, h, sw, sh);
                bmpFS.close();
                return;
            }

            uint32_t padding = (4 - ((w * 3) & 3)) & 3;
            uint32_t lineSize = (uint32_t)w * 3 + padding;

            // allocate on heap instead of stack to prevent stack overflow
            uint8_t *lineBuffer = (uint8_t *)malloc(lineSize);
            if (!lineBuffer)
            {
                Serial.println("Unable to allocate line buffer");
                bmpFS.close();
                return;
            }

            uint16_t *scaledLine = (uint16_t *)malloc(sizeof(uint16_t) * sw);
            if (!scaledLine)
            {
                Serial.println("Unable to allocate scaled line");
                free(lineBuffer);
                bmpFS.close();
                return;
            }

            for (uint16_t sRow = 0; sRow < sh; sRow++)
            {
                uint16_t orignalRow = sRow / scale;

                bmpFS.seek(seekOffset + (orignalRow * lineSize));
                bmpFS.read(lineBuffer, lineSize);

                for (uint16_t sCol = 0; sCol < sw; sCol++)
                {
                    uint16_t originalCol = sCol / scale;
                    uint8_t *bptr = &lineBuffer[originalCol * 3];

                    b = *bptr++;
                    g = *bptr++;
                    r = *bptr++;

                    scaledLine[sCol] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                tft.setSwapBytes(true);
                tft.pushImage(x, y + (sh - 1 - sRow), sw, 1, scaledLine);
            }

            free(lineBuffer);
            free(scaledLine);
        }
        else
        {
            Serial.println("BMP format not recognized.");
        }
    }
    bmpFS.close();
}

void drawIconWind(int rotation, int16_t x, int16_t y)
{
    const char *icons[] = {"direction.bmp", "direction_45.bmp", "direction_90.bmp", "direction_135.bmp",
                           "direction_180.bmp", "direction_225.bmp", "direction_270.bmp", "direction_315.bmp"};

    int normalized = (rotation % 360 + 360) % 360;
    int index = (normalized + 22) / 45 % 8;

    char fullPath[64];
    strcpy(fullPath, "/icons/");
    strcat(fullPath, icons[index]);
    drawBmp(fullPath, x, y);
}

void drawIconWeather(int weatherCode, int16_t x, int16_t y, bool wind, bool night, float scale)
{
    const char* image = "/icons/unknown.bmp";

    if (weatherCode <= 3)
    {
        // Indexed by [night][wind][weatherCode]
        static const char* const table[2][2][4] = {
            { // day
                { // no wind
                    "/icons/clear_day.bmp",
                    "/icons/mostly_clear_day.bmp",
                    "/icons/partly_cloudy_day.bmp",
                    "/icons/cloudy.bmp"
                },
                { // wind
                    "/icons/clear_day_wind.bmp",
                    "/icons/mostly_clear_day_wind.bmp",      
                    "/icons/partly_cloudy_day_wind.bmp",
                    "/icons/mostly_cloudy_day_wind.bmp"
                },
            },
            { // night
                { // no wind
                    "/icons/clear_night.bmp",
                    "/icons/mostly_clear_night.bmp",
                    "/icons/partly_cloudy_night.bmp",
                    "/icons/mostly_cloudy_night.bmp"
                },
                { // wind
                    "/icons/clear_night_wind.bmp",
                    "/icons/mostly_clear_night_wind.bmp",
                    "/icons/partly_cloudy_night_wind.bmp",
                    "/icons/mostly_cloudy_night_wind.bmp"
                    
                }
            }
        };
        image = table[night][wind][weatherCode];
    }
    else if (weatherCode == 45 || weatherCode == 48)  image = "/icons/fog.bmp";
    else if (weatherCode == 51 || weatherCode == 53 || weatherCode == 55) image = "/icons/drizzle.bmp";
    else if (weatherCode == 56 || weatherCode == 57)  image = "/icons/freezing_drizzle.bmp";
    else if (weatherCode == 66 || weatherCode == 67)  image = "/icons/freezing_rain.bmp";
    else if (weatherCode == 71 || weatherCode == 73 || weatherCode == 75) image = "/icons/snow.bmp";
    else if (weatherCode == 77)                       image = "/icons/snow_grains.bmp";
    else if (weatherCode == 80 || weatherCode == 81 || weatherCode == 82) image = "/icons/rain.bmp";
    else if (weatherCode == 85 || weatherCode == 86)  image = "/icons/snow_shower.bmp";
    else if (weatherCode == 95)                       image = "/icons/thunderstorm.bmp";
    else if (weatherCode == 96 || weatherCode == 99)  image = "/icons/thunderstorm_hail.bmp";

    drawBmp(image, x, y, scale);
}

void showWeather()
{
    tft.fillScreen(TFT_BLACK);

    printTimeTexts(true);
    printStaticTexts();
    printIcons();

    drawLineV(79, 0, (TFT_WIDTH * 2) / 3, TFT_DARKGREY);
    drawLineV(239, 0, (TFT_WIDTH * 2) / 3, TFT_DARKGREY);

    drawLineV(79, 159, TFT_WIDTH / 3);
    drawLineV(159, 159, TFT_WIDTH / 3);
    drawLineV(239, 159, TFT_WIDTH / 3);

    drawLineH(0, 159, TFT_HEIGHT);
    drawLineH(0, 160, TFT_HEIGHT);

    drawLineH(239, 39, TFT_HEIGHT / 4, TFT_DARKGREY);
    drawLineH(239, 79, TFT_HEIGHT / 4, TFT_DARKGREY);
    drawLineH(239, 119, TFT_HEIGHT / 4, TFT_DARKGREY);
}

void showSettings()
{
    tft.fillScreen(TFT_BLACK);
    delay(50);

    drawLineH(0, 39, TFT_HEIGHT);
    drawLineH(63, 105, TFT_HEIGHT * 3 / 5, TFT_DARKGREY);
    drawLineH(63, 171, TFT_HEIGHT * 3 / 5, TFT_DARKGREY);
    printSettingsTexts();

    drawBmp("/icons/settings.bmp", 295, 10);
}

void showAlarm()
{
    tft.fillScreen(TFT_BLACK);
    delay(50);

    drawLineH(0, 39, TFT_HEIGHT);
    drawLineH(63, 105, TFT_HEIGHT * 3 / 5, TFT_DARKGREY);
    drawLineH(63, 171, TFT_HEIGHT * 3 / 5, TFT_DARKGREY);
    printAlarmTexts();

    drawBmp("/icons/settings.bmp", 295, 10);
}