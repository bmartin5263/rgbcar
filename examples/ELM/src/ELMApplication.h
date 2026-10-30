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

class ELMApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {

  }

#if !RGB_NATIVE
  auto vehicleBackend() -> VehicleBackend<>& override {
    static auto parameters = std::array {
      ELMParameter<>::Rpm(),
      ELMParameter<>::Speed(),
      ELMParameter<>::CoolantTemperature(),
      ELMParameter<>::FuelLevel(),
      ELMParameter<>::ThrottlePosition(),
    };
    static auto backend = ELMBackend<>{PinNumber{RX}, PinNumber{TX}, parameters};
    return backend;
  }
#endif

  auto update() -> void override {
    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
      vehicle.logInformation();
    }
  }
};


#endif //RGBCAR_ELMAPPLICATION_H
