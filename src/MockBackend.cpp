//
// Created by Brandon on 9/20/26.
//

#include "MockBackend.h"

#include "Vehicle.h"


namespace rgb::car {

auto MockBackend::connect(Vehicle& vehicle) -> bool {
  vehicle.setConnected(true);
  return true;
}

auto MockBackend::isConnected() const -> bool {
  return true;
}

auto MockBackend::update(Vehicle& vehicle) -> void {
}

}
