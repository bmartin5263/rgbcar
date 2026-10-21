//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_ELMPROPERTY_H
#define RGBCAR_ELMPROPERTY_H

#include "OBD.h"
#include "Types.h"

namespace rgb::car {

class Vehicle;

using VehicleSetter = std::function<void(u32, Vehicle&)>;

struct ELMParameter {
  u8 pid{};
  VehicleSetter vehicleSetter{};

  static auto Rpm() -> ELMParameter;
  static auto Speed() -> ELMParameter;
  static auto CoolantTemperature() -> ELMParameter;
  static auto FuelLevel() -> ELMParameter;
  static auto ThrottlePosition() -> ELMParameter;
};

}

#endif //RGBCAR_ELMPROPERTY_H
