#ifndef BOOTBUTTON_H
#define BOOTBUTTON_H

#include "Moisture.h"

// GPIO9 is the BOOT button on the XIAO ESP32C3.
// The pin is pulled HIGH internally; pressing the button pulls it LOW.
#define BOOT_BUTTON_PIN     9
#define LONG_PRESS_MS    3000  // hold duration to trigger calibration reset

class BootButton {

public:
  BootButton(Moisture& moisture);
  void init();
  void loop();

private:
  enum State { RELEASED, PRESSING, TRIGGERED };

  Moisture&     _moisture;
  State         _state;
  unsigned long _pressedAt;
};

#endif // BOOTBUTTON_H
