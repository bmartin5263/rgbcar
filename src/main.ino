//
// Created by Brandon on 8/12/26.
//

#if RGB_ARDUINO_ESP32

#include "../examples/ELM/src/ELMApplication.h"
auto app = ELMApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}

#endif