//
// Created by Brandon on 8/17/26.
//

#if defined(RGB_NATIVE)

#include "RpmApplication.h"

auto main() -> int {
  auto app = RpmApplication{};
  app.run();
  return 0;
}

#endif // RGB_NATIVE