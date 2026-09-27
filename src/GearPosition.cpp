//
// Created by Brandon on 9/27/26.
//

#include "GearPosition.h"

namespace rgb::car {

auto ToString(GearPosition position) -> const char* {
  switch (position) {
    case GearPosition::P: return "P";
    case GearPosition::R: return "R";
    case GearPosition::N: return "N";
    case GearPosition::D: return "D";
    case GearPosition::L: return "L";
  }
  return "UNKNOWN";
}

}
