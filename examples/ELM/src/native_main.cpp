//
// Created by Brandon on 8/17/26.
//

#if RGB_NATIVE

#include "CANApplication.h"

auto main() -> int {
  auto app = CANApplication{};
  app.run();
  return 0;
}

#endif // RGB_NATIVE