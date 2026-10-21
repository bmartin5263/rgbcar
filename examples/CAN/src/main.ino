#if RGB_ARDUINO_ESP32

#include "CANApplication.h"

auto app = CANApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}

#endif //defined(RGB_ARDUINO_ESP32)
