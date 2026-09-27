#if RGB_ARDUINO_ESP32

#include "SniffApplication.h"

auto app = SniffApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}

#endif //defined(RGB_ARDUINO_ESP32)
