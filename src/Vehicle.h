//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_VEHICLE2_H
#define RGBCAR_VEHICLE2_H

#include <atomic>
#include <mutex>

#include "GearPosition.h"
#include "Util.h"

namespace rgb::car {

class Vehicle {
public:
  template<typename T>
  using TypeRemapper = T(*)(int);
  using recursive_mutex = std::recursive_mutex;
  template<class T>
  using atomic = std::atomic<T>;

  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);
  static constexpr auto READ_TIMEOUT_MS = 25;

  auto rpm() const -> revs_per_minute;
  auto speed() const -> kph;
  auto coolantTemp() const -> fahrenheit;
  auto fuelLevel() const -> percent;
  auto throttlePosition() const -> percent;
  auto gearNumber() const -> u8;
  auto gearPosition() const -> GearPosition;
  auto isBrakeApplied() const -> bool;
  auto isOverdriveActive() const -> bool;
  auto isTCSActive() const -> bool;
  auto isInfoButtonPressed() const -> bool;
  auto isSelectButtonPressed() const -> bool;
  auto isConnected() const -> bool;

  auto logInformation() const -> void;

  auto setRpm(revs_per_minute value) -> void;
  auto setSpeed(kph value) -> void;
  auto setCoolantTemp(fahrenheit value) -> void;
  auto setFuelLevel(percent value) -> void;
  auto setThrottlePosition(percent value) -> void;
  auto setGearNumber(u8 value) -> void;
  auto setGearPosition(GearPosition value) -> void;
  auto setBrakeApplied(bool value) -> void;
  auto setOverdriveActive(bool value) -> void;
  auto setTCSActive(bool value) -> void;
  auto setInfoButtonPressed(bool value) -> void;
  auto setSelectButtonPressed(bool value) -> void;
  auto setConnected(bool value) -> void;

private:
  atomic<revs_per_minute> mRpm{};
  atomic<kph> mSpeed{};
  atomic<fahrenheit> mCoolantTemp{};
  atomic<percent> mFuelLevel{};
  atomic<percent> mThrottlePosition{};
  atomic<u8> mGearNumber{};
  atomic<GearPosition> mGearPosition{};
  atomic<bool> mBrakeApplied{};
  atomic<bool> mOverdriveActive{true};
  atomic<bool> mTCSActive{true};
  atomic<bool> mInfoButtonPressed{};
  atomic<bool> mSelectButtonPressed{};
  atomic<bool> mConnected{false};
};

}

#endif //RGBCAR_VEHICLE2_H
