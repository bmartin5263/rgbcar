//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_BAUDRATE_H
#define RGBCAR_BAUDRATE_H

#include "Types.h"

namespace rgb::car {
enum class BaudRate : u32 {
  BAUD_125 = static_cast<u32>(125E3),
  BAUD_250 = static_cast<u32>(250E3),
  BAUD_500 = static_cast<u32>(500E3)
};
}

#endif //RGBCAR_BAUDRATE_H
