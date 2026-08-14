//
// Created by Brandon on 2/24/25.
//

#ifndef RGBLIB_VEHICLE_H
#define RGBLIB_VEHICLE_H

#include <OBD.h>
#include <atomic>
#include <mutex>
#include <bitset>
#include "Handle.h"
#include "Assertions.h"
#include "Clock.h"
#include "Util.h"
#include "Pin.h"
#include "VehicleUpdateCode.h"

#ifndef RGB_VEHICLE_CORE_STACK_SIZE
#define RGB_VEHICLE_CORE_STACK_SIZE 8192
#endif
#ifndef RGB_VEHICLE_CORE_PRIORITY
#define RGB_VEHICLE_CORE_PRIORITY 1
#endif
#ifndef RGB_VEHICLE_RX
#define RGB_VEHICLE_RX RX
#endif
#ifndef RGB_VEHICLE_TX
#define RGB_VEHICLE_TX TX
#endif

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

  auto update() -> VehicleUpdateCode;
  auto connect(PinNumber rx, PinNumber tx) -> bool;
  auto disconnect() -> void;
  auto setLowPowerMode(bool value) -> void;

  auto rpm() const -> revs_per_minute;
  auto speed() const -> kph;
  auto coolantTemp() const -> fahrenheit;
  auto fuelLevel() const -> percent;
  auto throttlePosition() const -> percent;
  auto inLowPowerMode() const -> bool;
  auto isConnected() const -> bool;

private:

  Handle<COBD, OBDDestroyer> obdHandle{{}};
  mutable recursive_mutex mu{};
  atomic<revs_per_minute> mRpm{};
  atomic<kph> mSpeed{};
  atomic<fahrenheit> mCoolantTemp{};
  atomic<percent> mFuelLevel{};
  atomic<percent> mThrottlePosition{};
  atomic<bool> mConnected{false};
  Timestamp mLastResponse{0};
  Timestamp mLastUpdate{0};
  atomic<bool> mLowPowerMode{false};

  constexpr static auto NoRemapping(int value) -> int { return value; }
  constexpr static auto ToPercent(int value) -> percent { return static_cast<float>(value) / 100.f; }
  constexpr static auto ToFloat(int value) -> float { return static_cast<float>(value); }
  constexpr static auto ToFahrenheit(int value) -> float { return CToF(static_cast<float>(value)); }
  constexpr static auto ToBitset(int value) -> std::bitset<32> { return std::bitset<32>(value); }

  template<typename T>
  auto readPID(byte pid, std::atomic<T>& result, TypeRemapper<T> remapper = NoRemapping) -> void {
    auto& obd = *obdHandle;
    ASSERT(obd.getState() == OBD_CONNECTED, "OBD not connected");

    int value;
    if (obd.readPID(pid, value, READ_TIMEOUT_MS)) {
      result = remapper(value);
      mLastResponse = Clock::Now();
    }
    else {
      if (Clock::Now().timeSince(mLastResponse) >= DISCONNECT_TIMEOUT) {
        disconnect();
      }
    }
  }
};

}

#endif //RGBLIB_VEHICLE_H
