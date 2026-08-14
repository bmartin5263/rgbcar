//
// Created by Brandon on 8/12/26.
//

#include "VehicleReader.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Log.h"
#include "Vehicle.h"
#include "VehicleLogger.h"

namespace rgb::car {
Vehicle* vehicleSingleton;

// Called from main thread
auto launchReader() -> void {
  auto vehicleLogger = VehicleLogger{};
  Debug::SetBlinker(BlinkerColor::YELLOW, [&] {
    return vehicleLogger.isStarted();
  });
  xTaskCreatePinnedToCore(vehicleReader2, "vehicleReader", RGB_VEHICLE_CORE_STACK_SIZE, &vehicleLogger, RGB_VEHICLE_CORE_PRIORITY, nullptr, 1);
}

[[noreturn]]
auto vehicleReader2(void* args) -> void {
  INFO("Vehicle Reader Task Started");

  auto vehicle = vehicleSingleton;
  auto logger = static_cast<VehicleLogger*>(args);

  vehicle->connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
  while (true) {
    if (!vehicle->isConnected()) {
      if (logger->isStarted()) {
        logger->flush();
      }
      vehicle->connect(PinNumber{RGB_VEHICLE_RX}, PinNumber{RGB_VEHICLE_TX});
      logger->start();
    }
    else {
      auto result = vehicle->update();
      if (logger->isStarted()) {
        logger->record(VehicleData{
          .lastUpdateResult = result,
          .rpm = vehicle->rpm(),
          .speed = vehicle->speed(),
          .coolantTemp = vehicle->coolantTemp(),
          .fuelLevel = vehicle->fuelLevel(),
          .throttlePosition = vehicle->throttlePosition(),
        });
      }
    }

    vTaskDelay(pdMS_TO_TICKS(70));
  }

}
}
