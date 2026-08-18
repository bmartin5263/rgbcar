//
// Created by Brandon on 8/17/26.
//

#if RGB_NATIVE

#include "../examples/Rpm/src/RpmApplication.h"
auto app = RpmApplication{};

auto main() -> int {
  app.run();
  return 0;
}

#endif //defined(RGB_NATIVE)
