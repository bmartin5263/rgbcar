//
// Created by Brandon on 8/17/26.
//

#if RGB_NATIVE

#include "SniffApplication.h"

auto main() -> int {
  auto app = SniffApplication{};
  app.run();
  return 0;
}

#endif // RGB_NATIVE