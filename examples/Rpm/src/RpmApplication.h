//
// Created by Brandon on 10/11/25.
//

#ifndef RGBLIB_EXAMPLEAPPLICATION_H
#define RGBLIB_EXAMPLEAPPLICATION_H

#include "LEDMatrix.h"
#include "LEDStrip.h"
#include "Log.h"
#include "VehicleApplication.h"
#include "Every.h"
#include "MockBackend.h"

using namespace rgb;
using namespace rgb::car;

inline auto strip = LEDStrip<40, D5_RGB>();
inline auto grid = LEDMatrix<8, 8, D2_RGB, RgbwSupport::ENABLE>();
inline auto connected = false;

#if RGB_NATIVE
#include "MockBackend.h"
inline auto& backend = MockBackend::Instance();
#else
#include "CANBackend.h"
inline auto backend = CANBackend{A0, CANBackend::ClockRate::MHZ_8};
#endif

class RpmApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {
    grid.setBrightness(.2f);
    app.addPixels(grid);
    app.addPixels(strip);

    app.on<VehicleConnected>([](auto&) {
      connected = true;
    });
    app.on<VehicleDisconnected>([](auto&) {
      connected = false;
    });
  }

  auto vehicleBackend() -> VehicleBackend* override {
    return &backend;
  }

  auto postDraw() -> void override {
    if (connected) {
      grid.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 9999.f));
      strip.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 9999.f));
    }
    else {
      grid.fill(Color::RED());
      strip.fill(Color::RED());
    }

    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
      backend.logInformation();
      vehicle.logInformation();
    }
  }
};


#endif //RGBLIB_EXAMPLEAPPLICATION_H
