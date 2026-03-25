#include <array>
#include <include/bedridden.hpp>

static int8_t activeFocus = -1;
static uint32_t pressStartTime = 0;
static bool lastPhysicalState = false;
static uint8_t lightValue = 70;
const uint8_t menuSizes[] = {2, 4, 5};

void moveFocus(std::array<std::array<Button, 5>, 3>& buttons, uint8_t menu)
{
  Button buttonOld = buttons[menu][activeFocus];
  tft.fillRect(buttonOld.x, buttonOld.y, buttonOld.w, buttonOld.h, TFT_BLACK);

  
  uint8_t maxButtons = menuSizes[menu];
  activeFocus = (activeFocus + 1) % maxButtons;

  Button& button = buttons[menu][activeFocus];
  tft.fillRect(button.x, button.y, button.w, button.h, TFT_DARKGREY);
  reprintAfterFocus(menu, activeFocus);
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
    static uint32_t lastInteractTick = 0;

    // --- Focus timeout ---
    if (activeFocus != -1 && (millis() - lastInteractTick >= 5000)) {
        Button& button = buttons[menu][activeFocus];
        tft.fillRect(button.x, button.y, button.w, button.h, TFT_BLACK);
        reprintAfterFocus(menu, activeFocus);
        activeFocus = -1;
    }

    // --- Edge detection ---
    bool pressed  = isPhysicallyPressed && !lastPhysicalState;
    bool released = !isPhysicallyPressed && lastPhysicalState;
    lastPhysicalState = isPhysicallyPressed;

    if (isPhysicallyPressed) pressStartTime = millis(); // track hold start

    if (pressed) {
        lastInteractTick = millis(); // reset timeout on any press
        moveFocus(buttons, menu);
    }

    // long press ~1s = execute
    if (released && (millis() - pressStartTime >= 1000)) {
        lastInteractTick = millis();
        executeButtonAction(menu);
    }
}