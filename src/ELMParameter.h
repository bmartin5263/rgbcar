//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_ELMPROPERTY_H
#define RGBCAR_ELMPROPERTY_H

#include <Util.h>

#include "OBD.h"
#include "Types.h"
#include "Vehicle.h"

namespace rgb::car {

class Vehicle;

template <typename Vehicle>
using VehicleSetter = std::function<void(u32, Vehicle&)>;

template <typename Vehicle = VehicleImpl>
struct ELMParameter {
  u8 pid{};
  VehicleSetter<Vehicle> vehicleSetter{};

  static auto Rpm() -> ELMParameter;
  static auto Speed() -> ELMParameter;
  static auto CoolantTemperature() -> ELMParameter;
  static auto FuelLevel() -> ELMParameter;
  static auto ThrottlePosition() -> ELMParameter;
};


template <typename Vehicle>
auto ELMParameter<Vehicle>::Rpm() -> ELMParameter {
  return ELMParameter{
    .pid = PID_RPM,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setRpm(result);
    }
  };
}

template <typename Vehicle>
auto ELMParameter<Vehicle>::Speed() -> ELMParameter {
  return ELMParameter{
    .pid = PID_SPEED,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setSpeed(result);
    }
  };
}

template <typename Vehicle>
auto ELMParameter<Vehicle>::CoolantTemperature() -> ELMParameter {
  return ELMParameter{
    .pid = PID_COOLANT_TEMP,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setCoolantTemp(CToF(static_cast<float>(result)));
    }
  };
}

template <typename Vehicle>
auto ELMParameter<Vehicle>::FuelLevel() -> ELMParameter {
  return ELMParameter{
    .pid = PID_FUEL_LEVEL,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setFuelLevel(static_cast<float>(result) / 100.f);
    }
  };
}

template <typename Vehicle>
auto ELMParameter<Vehicle>::ThrottlePosition() -> ELMParameter {
  return ELMParameter{
    .pid = PID_THROTTLE,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setThrottlePosition(static_cast<float>(result) / 100.f);
    }
  };
}

}

#endif //RGBCAR_ELMPROPERTY_H
