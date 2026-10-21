//
// Created by Brandon on 10/4/26.
//

#include "ELMParameter.h"
#include "Vehicle.h"

namespace rgb::car {

auto ELMParameter::Rpm() -> ELMParameter {
  return ELMParameter{
    .pid = PID_RPM,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setRpm(result);
    }
  };
}

auto ELMParameter::Speed() -> ELMParameter {
  return ELMParameter{
    .pid = PID_SPEED,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setSpeed(result);
    }
  };
}

auto ELMParameter::CoolantTemperature() -> ELMParameter {
  return ELMParameter{
    .pid = PID_COOLANT_TEMP,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setCoolantTemp(CToF(static_cast<float>(result)));
    }
  };
}

auto ELMParameter::FuelLevel() -> ELMParameter {
  return ELMParameter{
    .pid = PID_FUEL_LEVEL,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setFuelLevel(static_cast<float>(result) / 100.f);
    }
  };
}

auto ELMParameter::ThrottlePosition() -> ELMParameter {
  return ELMParameter{
    .pid = PID_THROTTLE,
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setThrottlePosition(static_cast<float>(result) / 100.f);
    }
  };
}

}
