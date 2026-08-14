//
// Created by Brandon on 8/13/26.
//

#include "VehicleMock.h"

namespace rgb::car {

auto VehicleMock::update() -> VehicleUpdateCode {
  return FULL;
}

auto VehicleMock::connect(PinNumber, PinNumber) -> bool {
  mConnected = true;
  return true;
}

auto VehicleMock::disconnect() -> void {
  mConnected = false;
}

auto VehicleMock::rpm() const -> revs_per_minute {
  return mRpm;
}

auto VehicleMock::speed() const -> kph {
  return mSpeed;
}

auto VehicleMock::coolantTemp() const -> fahrenheit {
  return mCoolantTemp;
}

auto VehicleMock::fuelLevel() const -> percent {
  return mFuelLevel;
}

auto VehicleMock::throttlePosition() const -> percent {
  return mThrottlePosition;
}

auto VehicleMock::inLowPowerMode() const -> bool {
  return mLowPowerMode;
}

auto VehicleMock::isConnected() const -> bool {
  return mConnected;
}

auto VehicleMock::setRpm(revs_per_minute value) -> void {
  mRpm = value;
}

auto VehicleMock::setSpeed(kph value) -> void {
  mSpeed = value;
}

auto VehicleMock::setCoolantTemp(fahrenheit value) -> void {
  mCoolantTemp = value;
}

auto VehicleMock::setFuelLevel(percent value) -> void {
  mFuelLevel = value;
}

auto VehicleMock::setThrottlePosition(percent value) -> void {
  mThrottlePosition = value;
}

auto VehicleMock::setLowPowerMode(bool value) -> void {
  mLowPowerMode = value;
}

}
