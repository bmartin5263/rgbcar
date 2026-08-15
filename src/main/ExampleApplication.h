//
// Created by Brandon on 10/11/25.
//

#ifndef RGBLIB_EXAMPLEAPPLICATION_H
#define RGBLIB_EXAMPLEAPPLICATION_H

#include "FastLEDMatrix.h"
#include "FastLEDStrip.h"
#include "Every.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

inline auto strip = FastLEDStrip<40, D5_RGB>();
inline auto grid = FastLEDMatrix<8, 8, D2_RGB, RgbwSupport::ENABLE>();

inline auto firstConnection = false;

class ExampleApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {
    grid.setBrightness(.2f);
    app.addLEDs(grid);
    app.addLEDs(strip);

    app.on<VehicleConnected>([](auto& event) {
      firstConnection = true;
    });
  }

  auto update() -> void override {
  }

  auto draw() -> void override {
  }

  auto postDraw() -> void override {
    if (firstConnection) {
      grid.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 9999.f));
      strip.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 9999.f));
    }
    else {
      grid.fill(Color::RED());
      strip.fill(Color::RED());
    }
  }
};


#endif //RGBLIB_EXAMPLEAPPLICATION_H
