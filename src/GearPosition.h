//
// Created by Brandon on 9/21/26.
//

#ifndef RGBCAR_GEARPOSITION_H
#define RGBCAR_GEARPOSITION_H

namespace rgb::car {

enum class GearPosition {
  P, R, N, D, L
};

auto ToString(GearPosition position) -> const char*;
auto ToChar(GearPosition position) -> char;

}

#endif //RGBCAR_GEARPOSITION_H
