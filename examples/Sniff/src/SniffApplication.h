//
// Created by Brandon on 10/11/25.
//

#ifndef RGBCAR_SNIFFAPPLICATION_H
#define RGBCAR_SNIFFAPPLICATION_H

#include "CANBackend.h"
#include "Every.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

#if RGB_NATIVE
#include "MockBackend.h"
inline auto& backend = MockBackend::Instance();
#else
#include "CANBackend.h"
inline auto backend = CANBackend{A0, CANBackend::ClockRate::MHZ_8};
#endif

class SniffApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {

  }

  auto vehicleBackend() -> VehicleBackend& override {
    return backend;
  }

  auto update() -> void override {
    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
      backend.logInformation();
      vehicle.logInformation();
    }
  }
};


#endif //RGBCAR_SNIFFAPPLICATION_H
