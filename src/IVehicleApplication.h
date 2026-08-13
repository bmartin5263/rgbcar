//
// Created by Brandon on 8/13/26.
//

#ifndef RGBCAR_IVEHICLEAPPLICATION_H
#define RGBCAR_IVEHICLEAPPLICATION_H

#include "VehicleEvents.h"

namespace rgb::car {
class IVehicleApplication {
public:
  static IVehicleApplication* instance;
  virtual auto publishVehicleEvent(const VehicleEvents& event) -> void = 0;

  template<typename T>
  static auto PublishVehicleEvent(const T& event) -> void {
    instance->publishVehicleEvent(VehicleEvents{event});
  }

  virtual ~IVehicleApplication() = default;
  IVehicleApplication(const IVehicleApplication& rhs) = delete;
  IVehicleApplication(IVehicleApplication&& rhs) noexcept = delete;
  IVehicleApplication& operator=(const IVehicleApplication& rhs) = delete;
  IVehicleApplication& operator=(IVehicleApplication&& rhs) noexcept = delete;
protected:
  IVehicleApplication() = default;
};

}

#endif //RGBCAR_IVEHICLEAPPLICATION_H
