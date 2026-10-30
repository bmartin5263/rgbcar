//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_MOCKBACKEND_H
#define RGBCAR_MOCKBACKEND_H

#include "VehicleBackend.h"

namespace rgb::car {

template <typename Vehicle>
class MockBackend : public VehicleBackend<Vehicle> {
public:
  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;

  MockBackend(const MockBackend& rhs) = delete;
  MockBackend(MockBackend&& rhs) noexcept = delete;
  MockBackend& operator=(const MockBackend& rhs) = delete;
  MockBackend& operator=(MockBackend&& rhs) noexcept = delete;
private:
  MockBackend() = default;
  ~MockBackend() override = default;
};

template <typename Vehicle>
auto MockBackend<Vehicle>::connect(Vehicle& vehicle) -> bool {
  vehicle.setConnected(true);
  return true;
}

template <typename Vehicle>
auto MockBackend<Vehicle>::isConnected() const -> bool {
  return true;
}

template <typename Vehicle>
auto MockBackend<Vehicle>::update(Vehicle& vehicle) -> void {
}

}

#endif //RGBCAR_MOCKBACKEND_H
