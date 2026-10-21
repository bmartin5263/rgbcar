//
// Created by Brandon on 10/11/25.
//

#ifndef RGBCAR_ELMAPPLICATION_H
#define RGBCAR_ELMAPPLICATION_H

#include "ELMBackend.h"
#include "Every.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

#if RGB_NATIVE
#include "MockBackend.h"
inline auto& backend = MockBackend::Instance();
#else
#include "ELMBackend.h"
inline auto parameters = std::array {
  ELMParameter::Rpm(),
  ELMParameter::Speed(),
  ELMParameter::CoolantTemperature(),
  ELMParameter::FuelLevel(),
  ELMParameter::ThrottlePosition(),
};
inline auto backend = ELMBackend{PinNumber{RX}, PinNumber{TX}, parameters};
#endif

class ELMApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {

  }

  auto vehicleBackend() -> VehicleBackend& override {
    return backend;
  }

  auto update() -> void override {
    auto& vehicle = Vehicle::Instance();
    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
      vehicle.logInformation();
    }
  }
};


#endif //RGBCAR_ELMAPPLICATION_H
