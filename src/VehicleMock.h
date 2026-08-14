//
// Created by Brandon on 2/24/25.
//

#ifndef RGBLIB_VEHICLE_MOCK_H
#define RGBLIB_VEHICLE_MOCK_H

#include <atomic>
#include <mutex>
#include "Pin.h"
#include "VehicleUpdateCode.h"

namespace rgb::car {

class VehicleMock {
public:
  template<typename T> using TypeRemapper = T(*)(int);
  using recursive_mutex = std::recursive_mutex;
  template<class T> using atomic = std::atomic<T>;

  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);

  auto update() -> VehicleUpdateCode;
  auto connect(PinNumber, PinNumber) -> bool;
  auto disconnect() -> void;

  auto rpm() const -> revs_per_minute;
  auto speed() const -> kph;
  auto coolantTemp() const -> fahrenheit;
  auto fuelLevel() const -> percent;
  auto throttlePosition() const -> percent;
  auto inLowPowerMode() const -> bool;
  auto isConnected() const -> bool;

  auto setRpm(revs_per_minute value) -> void;
  auto setSpeed(kph value) -> void;
  auto setCoolantTemp(fahrenheit value) -> void;
  auto setFuelLevel(percent value) -> void;
  auto setThrottlePosition(percent value) -> void;
  auto setLowPowerMode(bool value) -> void;

private:
  mutable recursive_mutex mu{};
  atomic<revs_per_minute> mRpm{};
  atomic<kph> mSpeed{};
  atomic<fahrenheit> mCoolantTemp{};
  atomic<percent> mFuelLevel{};
  atomic<percent> mThrottlePosition{};
  atomic<bool> mConnected{false};
  atomic<bool> mLowPowerMode{false};
};

}

#endif //RGBLIB_VEHICLE_MOCK_H
