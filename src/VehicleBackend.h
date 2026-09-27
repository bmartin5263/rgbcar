//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_VEHICLEBACKEND_H
#define RGBCAR_VEHICLEBACKEND_H

namespace rgb::car {
class Vehicle;
class VehicleBackend {
public:
  VehicleBackend() = default;
  VehicleBackend(const VehicleBackend& rhs) = default;
  VehicleBackend(VehicleBackend&& rhs) noexcept = default;
  VehicleBackend& operator=(const VehicleBackend& rhs) = default;
  VehicleBackend& operator=(VehicleBackend&& rhs) noexcept = default;
  virtual ~VehicleBackend() = default;

  virtual auto connect(Vehicle& vehicle) -> bool = 0;
  virtual auto isConnected() const -> bool = 0;
  virtual auto update(Vehicle& vehicle) -> void = 0;
};
}

#endif //RGBCAR_VEHICLEBACKEND_H
