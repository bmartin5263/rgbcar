//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_MOCKBACKEND_H
#define RGBCAR_MOCKBACKEND_H
#include "VehicleBackend.h"


namespace rgb::car {

class MockBackend : public VehicleBackend {
public:
  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;

  static auto Instance() -> MockBackend& {
    static MockBackend instance;
    return instance;
  }

  MockBackend(const MockBackend& rhs) = delete;
  MockBackend(MockBackend&& rhs) noexcept = delete;
  MockBackend& operator=(const MockBackend& rhs) = delete;
  MockBackend& operator=(MockBackend&& rhs) noexcept = delete;
private:
  MockBackend() = default;
  ~MockBackend() override = default;
};
}

#endif //RGBCAR_MOCKBACKEND_H
