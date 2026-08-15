#include "RpmApplication.h"

auto app = RpmApplication{};

auto setup() -> void {
  app.setup();
}

auto loop() -> void {
  app.loop();
}
