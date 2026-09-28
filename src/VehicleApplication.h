//
// Created by Brandon on 8/12/26.
//

#ifndef RGBCAR_VEHICLEAPPLICATION_H
#define RGBCAR_VEHICLEAPPLICATION_H

#include "IVehicleApplication.h"
#include <UserApplication.h>

#include "CANBackend.h"
#include "VehicleLogger.h"
#include "VehicleEvents.h"
#include "VehicleBackend.h"
#include "Vehicle.h"

#if RGB_NATIVE
#include "VehicleControlPanel.h"
#endif

namespace rgb::car {

template<typename EventVariantT = VehicleEvents>
class VehicleApplication : public UserApplication<EventVariantT>, public IVehicleApplication {

public:
  using AnyEvent = typename UserApplication<EventVariantT>::AnyEvent; // typename is required
  auto publishVehicleEvent(const VehicleEvents& vehicleEvent) -> void override;

protected:
  using UserApplication<EventVariantT>::mEventMap;

  auto initialize() -> void override;
  virtual auto vehicleBackend() -> VehicleBackend& = 0;

#if RGB_NATIVE
  // Ticks the vehicle control panel. Subclasses overriding update()
  // must call VehicleApplication::update() themselves to keep it running.
  auto update() -> void override;
#endif

private:
#if  RGB_ARDUINO_ESP32
  static auto VehicleTaskStatic(void* params) -> void;
  auto vehicleTask() -> void;
#endif

protected:
#if RGB_ARDUINO_ESP32
  VehicleLogger logger;
#endif
#if RGB_NATIVE
  VehicleControlPanel controlPanel;
#endif
};

template<typename EventVariantT>
auto VehicleApplication<EventVariantT>::publishVehicleEvent(const VehicleEvents& vehicleEvent) -> void {
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

template<typename EventVariantT>
void VehicleApplication<EventVariantT>::initialize() {
  instance = this;
  // Debug::SetBlinker(BlinkerColor::GREEN, [this] {
  //   return vehicle.isConnected();
  // });
#if RGB_ARDUINO_ESP32
  Debug::SetBlinker(BlinkerColor::YELLOW, [this] {
    return logger.isStarted();
  });
  xTaskCreatePinnedToCore(VehicleTaskStatic, "vehicleReader", RGB_VEHICLE_CORE_STACK_SIZE, this, RGB_VEHICLE_CORE_PRIORITY, nullptr, 1);
#endif
}

#if RGB_NATIVE
template<typename EventVariantT>
auto VehicleApplication<EventVariantT>::update() -> void {
  controlPanel.update(vehicle);
}
#endif

#if RGB_ARDUINO_ESP32
template<typename EventVariantT>
auto VehicleApplication<EventVariantT>::VehicleTaskStatic(void* params) -> void {
  static_cast<VehicleApplication*>(params)->vehicleTask();
}

// template<typename EventVariantT>
// auto VehicleApplication<EventVariantT>::vehicleTask() -> void {
//   INFO("Vehicle Reader Task Started");
//
//   vehicle.connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
//   while (true) {
//     if (!vehicle.isConnected()) {
//       if (logger.isStarted()) {
//         logger.flush();
//       }
//       vehicle.connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
//       logger.start();
//     }
//     else {
//       auto result = vehicle.update();
//       if (logger.isStarted()) {
//         logger.record(car::VehicleData{
//           .lastUpdateResult = result,
//           .rpm = vehicle.rpm(),
//           .speed = vehicle.speed(),
//           .coolantTemp = vehicle.coolantTemp(),
//           .fuelLevel = vehicle.fuelLevel(),
//           .throttlePosition = vehicle.throttlePosition(),
//         });
//       }
//     }
//
//     vTaskDelay(pdMS_TO_TICKS(70));
//   }
// }

template<typename EventVariantT>
auto VehicleApplication<EventVariantT>::vehicleTask() -> void {
  auto& backend = vehicleBackend();
  auto& vehicle = Vehicle::Instance();
  INFO("Vehicle Reader Task Started");
  while (true) {
    if (!backend.isConnected()) {
      backend.connect(vehicle);
    }
    else {
      backend.update(vehicle);
    }
    vTaskDelay(1);
  }
}
#endif


}

#endif //RGBCAR_VEHICLEAPPLICATION_H
