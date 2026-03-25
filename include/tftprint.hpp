#pragma once
#include <TFT_eSPI.h>
#include <include/bedridden.hpp>

extern TFT_eSPI tft;
extern bool menuNotChanged;
class TFTPrint
{
private:
    uint16_t cursorX, cursorY;
    uint8_t size, width;
    uint16_t color, backgroundColor;
    char lastText[16];

public:
    TFTPrint() : cursorX(0), cursorY(0), size(1), width(0)
    {
        lastText[0] = '\0';
    }

    void begin(uint16_t _x, uint16_t _y, uint8_t _size, uint16_t _color, uint16_t _bg)
    {
        cursorX = _x;
        cursorY = _y;
        size = _size;
        color = _color;
        backgroundColor = _bg;
        width = 0;
        lastText[0] = '\0';
    }

    void begin(uint16_t x1, uint16_t x2, uint16_t _y, uint8_t _size, uint16_t _color, uint16_t _bg)
    {
        cursorX = x1;
        cursorY = _y;
        size = _size;
        color = _color;
        backgroundColor = _bg;
        width = x2 - x1;
        lastText[0] = '\0';
    }

    template <typename T>
    void Print(const T value, uint16_t customBackColor = 0)
    {
        char currentText[32];
        snprintf(currentText, sizeof(currentText), "%s", String(value).c_str());

        if (strcmp(lastText, currentText) == 0 && menuNotChanged)
            return;

        tft.setTextSize(size);
        
        if (customBackColor == 0) tft.setTextColor(color, backgroundColor);
        else tft.setTextColor(color, customBackColor);

        tft.setTextDatum(TL_DATUM);

        uint16_t currentW = tft.textWidth(currentText);
        uint16_t lastW = tft.textWidth(lastText);
        uint16_t padW = (currentW > lastW) ? currentW : lastW;
        tft.setTextPadding(padW);

        tft.drawString(currentText, cursorX, cursorY);

        strncpy(lastText, currentText, sizeof(lastText));
    }

    template <typename T>
    void PrintCentered(const T value, uint16_t customBackColor = 0)
    {
        char currentText[32];
        snprintf(currentText, sizeof(currentText), "%s", String(value).c_str());

        if (strcmp(lastText, currentText) == 0 && menuNotChanged)
            return;

        tft.setTextSize(size);

        if (customBackColor == 0) tft.setTextColor(color, backgroundColor);
        else tft.setTextColor(color, customBackColor);

        tft.setTextDatum(TC_DATUM);

        tft.setTextPadding(width > 0 ? width : tft.textWidth(lastText));

        tft.drawString(currentText, cursorX + (width / 2), cursorY);

        strncpy(lastText, currentText, sizeof(lastText));
    }

    void forceRedraw() 
    {
        lastText[0] = '\0';
    }
};