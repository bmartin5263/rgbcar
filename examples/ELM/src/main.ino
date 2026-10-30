#if RGB_ARDUINO_ESP32

#include "ELMApplication.h"

auto app = ELMApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}

#endif //defined(RGB_ARDUINO_ESP32)
