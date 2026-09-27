//
// Created by Brandon on 10/11/25.
//

#ifndef RGBCAR_SNIFFAPPLICATION_H
#define RGBCAR_SNIFFAPPLICATION_H

#include "CANModule.h"
#include "LEDMatrix.h"
#include "LEDStrip.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

inline auto mcp2515 = CANModule{A0, CANModule::ClockRate::MHZ_8};

class SniffApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {

  }

  auto update() -> void override {
    if (!mcp2515.isConnected()) {
      mcp2515.connect();
    }
    else {
      mcp2515.request();
      mcp2515.update();
    }
  }
};


#endif //RGBCAR_SNIFFAPPLICATION_H
