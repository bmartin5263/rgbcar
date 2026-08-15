//
// Created by Brandon on 8/12/26.
//

#ifndef RGBCAR_VEHICLEAPPLICATION_H
#define RGBCAR_VEHICLEAPPLICATION_H

#include "IVehicleApplication.h"
#include <UserApplication.h>
#include "Vehicle.h"
#include "VehicleLogger.h"
#include "VehicleEvents.h"

namespace rgb::car {

template<typename EventVariantT = VehicleEvents, typename VehicleImpl = Vehicle>
class VehicleApplication : public UserApplication<EventVariantT>, public IVehicleApplication {

public:
  using AnyEvent = typename UserApplication<EventVariantT>::AnyEvent;
  auto publishVehicleEvent(const VehicleEvents& vehicleEvent) -> void override;

protected:
  using UserApplication<EventVariantT>::mEventMap;
  auto initialize() -> void override;

private:
  static auto VehicleTaskStatic(void* params) -> void;
  auto vehicleTask() -> void;

protected:
  VehicleImpl vehicle;
  VehicleLogger logger;
};

template<typename EventVariantT, typename VehicleImpl>
auto VehicleApplication<EventVariantT, VehicleImpl>::publishVehicleEvent(const VehicleEvents& vehicleEvent) -> void {
  auto event = std::visit([](auto&& e) {
  return AnyEvent{e};
}, vehicleEvent);
  auto uid = vehicleEvent.index();
  if (auto it = mEventMap.find(uid); it != mEventMap.end()) {
    for (auto& handler : it->second) {
      handler(event);
    }
  }
}

template<typename EventVariantT, typename VehicleImpl>
void VehicleApplication<EventVariantT, VehicleImpl>::initialize() {
  instance = this;
  Debug::SetBlinker(BlinkerColor::GREEN, [this] {
    return vehicle.isConnected();
  });
  Debug::SetBlinker(BlinkerColor::YELLOW, [this] {
    return logger.isStarted();
  });
  xTaskCreatePinnedToCore(VehicleTaskStatic, "vehicleReader", RGB_VEHICLE_CORE_STACK_SIZE, this, RGB_VEHICLE_CORE_PRIORITY, nullptr, 1);
}

template<typename EventVariantT, typename VehicleImpl>
auto VehicleApplication<EventVariantT, VehicleImpl>::VehicleTaskStatic(void* params) -> void {
  static_cast<VehicleApplication*>(params)->vehicleTask();
}

template<typename EventVariantT, typename VehicleImpl>
auto VehicleApplication<EventVariantT, VehicleImpl>::vehicleTask() -> void {
  INFO("Vehicle Reader Task Started");

  vehicle.connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
  while (true) {
    if (!vehicle.isConnected()) {
      if (logger.isStarted()) {
        logger.flush();
      }
      vehicle.connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
      logger.start();
    }
    else {
      auto result = vehicle.update();
      if (logger.isStarted()) {
        logger.record(car::VehicleData{
          .lastUpdateResult = result,
          .rpm = vehicle.rpm(),
          .speed = vehicle.speed(),
          .coolantTemp = vehicle.coolantTemp(),
          .fuelLevel = vehicle.fuelLevel(),
          .throttlePosition = vehicle.throttlePosition(),
        });
      }
    }

    vTaskDelay(pdMS_TO_TICKS(70));
  }
}


}

#endif //RGBCAR_VEHICLEAPPLICATION_H
