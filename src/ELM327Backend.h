//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_OBDBACKEND_H
#define RGBCAR_OBDBACKEND_H

#if !RGB_NATIVE

#include <Assertions.h>
#include <Handle.h>
#include <mutex>
#include <Pin.h>

#include "OBD.h"
#include "VehicleBackend.h"
#include "Vehicle.h"

namespace rgb::car {

class ELM327Backend : public VehicleBackend {
public:
  template<typename T>
  using TypeRemapper = T(*)(int);
  using recursive_mutex = std::recursive_mutex;

  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);
  static constexpr auto READ_TIMEOUT_MS = 25;

  enum class PID {
    RPM=PID_RPM,
    SPEED=PID_SPEED,
    COOLANT_TEMP=PID_COOLANT_TEMP,
    FUEL_LEVEL=PID_FUEL_LEVEL,
    THROTTLE_POSITION=PID_THROTTLE,

    Count_ = 5
  };

  using VehicleSetter = std::function<void(u32, Vehicle&)>;

  struct Property {
    PID pid{};
    VehicleSetter vehicleSetter{};
    Duration frequency{Duration::Milliseconds(200)};
    Timestamp lastRequestedAt{};
    Timestamp lastReceivedAt{};
    Duration averageResponseTime{};
    uint sentMessages{};
    uint droppedMessages{};
    uint sendFailures{};
  };

  ELM327Backend(PinNumber rx, PinNumber tx)
    : mObdHandle{{}}, mLastResponse{}, mLastUpdate{}, mRx(rx), mTx(tx), mConnected(false) {
  }

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto disconnect(Vehicle& vehicle) -> void;

private:
  Handle<COBD, OBDDestroyer> mObdHandle;
  Timestamp mLastResponse;
  Timestamp mLastUpdate;
  PinNumber mRx;
  PinNumber mTx;
  bool mConnected;

  std::array<Property, static_cast<int>(PID::Count_)> properties = std::array {
    Property{
      .pid = PID::RPM,
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setRpm(result);
      }
    },
    Property{
      .pid = PID::SPEED,
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setSpeed(result);
      }
    },
    Property{
      .pid = PID::COOLANT_TEMP,
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setCoolantTemp(CToF(static_cast<float>(result)));
      }
    },
    Property{
      .pid = PID::FUEL_LEVEL,
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setFuelLevel(static_cast<float>(result) / 100.f);
      }
    },
    Property{
      .pid = PID::THROTTLE_POSITION,
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setThrottlePosition(static_cast<float>(result) / 100.f);
      }
    },
  };

  auto readPID(byte pid, Vehicle& vehicle, const VehicleSetter& vehicleSetter) -> void;
};

}

#endif //RGBCAR_OBDBACKEND_H
#endif