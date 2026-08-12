//
// Created by Brandon on 3/29/26.
//

#ifndef RGBLIB_VEHICLEDATA_H
#define RGBLIB_VEHICLEDATA_H

#include "Types.h"
#include "VehicleUpdateCode.h"

namespace rgb::car {

struct VehicleData {
  VehicleUpdateCode lastUpdateResult{};
  i32 rpm{};
  i32 speed{};
  f32 coolantTemp{};
  f32 fuelLevel{};
  f32 throttlePosition{};
};

}

#endif //RGBLIB_VEHICLEDATA_H
