#if defined(RGB_ARDUINO_ESP32)

#include "RpmApplication.h"

auto app = RpmApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}

#endif //defined(RGB_ARDUINO_ESP32)
