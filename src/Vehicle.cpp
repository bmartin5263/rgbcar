//
// Created by Brandon on 9/20/26.
//

#include "Vehicle.h"

#include <Clock.h>
#include <Log.h>

#include "IVehicleApplication.h"

namespace rgb::car {

auto VehicleImpl::rpm() const -> revs_per_minute {
  return mRpm;
}

auto VehicleImpl::speed() const -> kph {
  return mSpeed;
}

auto VehicleImpl::coolantTemp() const -> fahrenheit {
  return mCoolantTemp;
}

auto VehicleImpl::fuelLevel() const -> percent {
  return mFuelLevel;
}

auto VehicleImpl::throttlePosition() const -> percent {
  return mThrottlePosition;
}

auto VehicleImpl::isConnected() const -> bool {
  return mConnected;
}

auto VehicleImpl::logInformation() const -> void {
  static constexpr auto BoolToString = [](bool value) -> const char* { return value ? "true" : "false"; };

  log::printMessage("%-22s | %s", "Parameter", "Value");
  log::printMessage("-------------------------------------");
  log::printMessage("%-22s | %d rpm", "RPM", rpm());
  log::printMessage("%-22s | %d mph", "Speed", ToMph(speed()));
  log::printMessage("%-22s | %.1f F", "Coolant Temp", coolantTemp());
  log::printMessage("%-22s | %.1f%%", "Fuel Level", fuelLevel());
  log::printMessage("%-22s | %.1f%%", "Throttle Position", throttlePosition());
  log::printMessage("%-22s | %s", "Connected", BoolToString(isConnected()));
}

auto VehicleImpl::setRpm(revs_per_minute value) -> void {
  mRpm = value;
}

auto VehicleImpl::setSpeed(kph value) -> void {
  mSpeed = value;
}

auto VehicleImpl::setCoolantTemp(fahrenheit value) -> void {
  mCoolantTemp = value;
}

auto VehicleImpl::setFuelLevel(percent value) -> void {
  mFuelLevel = value;
}

auto VehicleImpl::setThrottlePosition(percent value) -> void {
  mThrottlePosition = value;
}

auto VehicleImpl::setConnected(bool value) -> void {
  if (auto prev = mConnected.exchange(value); prev != value) {
    if (value) {
      IVehicleApplication::PublishVehicleEvent(VehicleConnected{{Clock::Now()}});
    }
    else {
      IVehicleApplication::PublishVehicleEvent(VehicleDisconnected{{Clock::Now()}});
    }
  }
}
}
