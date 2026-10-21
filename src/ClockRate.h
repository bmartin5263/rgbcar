//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_CLOCKRATE_H
#define RGBCAR_CLOCKRATE_H

#include "Types.h"

namespace rgb::car {
enum class ClockRate : u32 {
  MHZ_8 = static_cast<u32>(8E6),
  MHZ_16 = static_cast<u32>(16E6),
  MHZ_25 = static_cast<u32>(25E6),
  MHZ_40 = static_cast<u32>(40E6)
};
}

#endif //RGBCAR_CLOCKRATE_H
