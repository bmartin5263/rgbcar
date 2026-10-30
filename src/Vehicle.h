//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_VEHICLE2_H
#define RGBCAR_VEHICLE2_H

#include <atomic>
#include "Util.h"

namespace rgb::car {

class VehicleImpl {
public:
  template<class T>
  using atomic = std::atomic<T>;

  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);
  static constexpr auto READ_TIMEOUT_MS = 25;

  auto rpm() const -> revs_per_minute;
  auto speed() const -> kph;
  auto coolantTemp() const -> fahrenheit;
  auto fuelLevel() const -> percent;
  auto throttlePosition() const -> percent;
  auto isConnected() const -> bool;

  virtual auto logInformation() const -> void;

  auto setRpm(revs_per_minute value) -> void;
  auto setSpeed(kph value) -> void;
  auto setCoolantTemp(fahrenheit value) -> void;
  auto setFuelLevel(percent value) -> void;
  auto setThrottlePosition(percent value) -> void;
  auto setConnected(bool value) -> void;

  VehicleImpl() = default;
  virtual ~VehicleImpl() = default;

  VehicleImpl(const VehicleImpl& rhs) = delete;
  VehicleImpl(VehicleImpl&& rhs) noexcept = delete;
  VehicleImpl& operator=(const VehicleImpl& rhs) = delete;
  VehicleImpl& operator=(VehicleImpl&& rhs) noexcept = delete;
private:
  atomic<revs_per_minute> mRpm{};
  atomic<kph> mSpeed{};
  atomic<fahrenheit> mCoolantTemp{};
  atomic<percent> mFuelLevel{};
  atomic<percent> mThrottlePosition{};
  atomic<bool> mConnected{false};
};

}

#endif //RGBCAR_VEHICLE2_H
