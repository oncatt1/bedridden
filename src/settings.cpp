#include <array>
#include <include/bedridden.hpp>

static uint8_t activeFocus = 0;
static uint32_t pressStartTime = 0;
static bool lastPhysicalState = false;
static uint8_t lightValue = 70;

void moveFocus(std::array<std::array<Button, 5>, 3>& buttons, uint8_t menu)
{
  Button buttonOld = buttons[menu][activeFocus];
  tft.drawRect(buttonOld.x, buttonOld.y, buttonOld.w, buttonOld.h, TFT_BLACK);
  buttonOld.isFocused = false;

  uint8_t maxButtons = 0;
  switch (menu) {
    case 0: maxButtons = 2;
    case 1: maxButtons = 4;
    case 2: maxButtons = 5;
  }
  activeFocus = (activeFocus + 1) % maxButtons;

  Button button = buttons[menu][activeFocus];
  tft.drawRect(button.x, button.y, button.w, button.h, TFT_DARKGREY);
  button.isFocused = true;
}

void executeButtonAction(uint8_t& menu) {
  if((menu == 1 || menu == 2 ) && activeFocus == 0){ 
    forceSensors = true;
    menuNotChanged = false;
    menu = 0;
    activeFocus = 0;
    showWeather();
    return;
  }

  if (menu == 0) { // Weather
    if (activeFocus == 0) {
      menu = 1;
      showSettings();
    }
    else if (activeFocus == 1) {
      menu = 2;
      showAlarm();
    }
    menuNotChanged = false;
    activeFocus = 0;
  }
  else if (menu == 1) { // Settings
    if (activeFocus == 1) {
      lightValue = (lightValue + 10) % 100;
      analogWrite(TFT_BL, map(lightValue, 0, 100, 0, 255));
    }
    else if (activeFocus == 2) forceSensors = true;
    else if (activeFocus == 3) esp_restart();
  }
  else if (menu == 2) { // Alarm
    if (activeFocus == 1) return;
  }
}

void handleInput(std::array<std::array<Button, 5>, 3>& buttons, uint8_t& menu, bool isPhysicallyPressed) {
  
  if (isPhysicallyPressed && !lastPhysicalState) {
    pressStartTime = millis();
  }

  if (!isPhysicallyPressed && lastPhysicalState) {
    uint32_t duration = millis() - pressStartTime;

    if (duration > 50 && duration < 1000) {
      Serial.println("short click");
      moveFocus(buttons, menu);
    } 
    else if (duration >= 1000) { 
      Serial.println("long click");
      executeButtonAction(menu);
    }
  }

  lastPhysicalState = isPhysicallyPressed;
}