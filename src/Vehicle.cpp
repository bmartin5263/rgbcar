//
// Created by Brandon on 2/24/25.
//

#include "Vehicle.h"
#include "Clock.h"
#include "IVehicleApplication.h"

namespace rgb::car {

auto Vehicle::connect(PinNumber rx, PinNumber tx) -> bool {
  auto lock = std::unique_lock { mu };
  if (obdHandle->getState() == OBD_CONNECTED) {
    INFO("Vehicle already connected");
    mConnected = true;
    return true;
  }

  TRACE("Connecting");

  obdHandle.reset({});
  mConnected = false;

  if (!obdHandle->begin(rx.to<i8>(), tx.to<i8>())) {
    ERROR("Vehicle begin() failed with Pins RX=%i, TX=%i", rx.to<i8>(), tx.to<i8>());
    return false;
  }

  if (!obdHandle->init()) {
    ERROR("Vehicle init() failed with Pins RX=%i, TX=%i", rx.to<i8>(), tx.to<i8>());
    return false;
  }

  INFO("Vehicle ready");
  mConnected = true;
  mLastResponse = Clock::Now();

  IVehicleApplication::PublishVehicleEvent(VehicleConnected{Clock::Now()});

  return true;
}

auto Vehicle::disconnect() -> void {
  auto lock = std::unique_lock { mu };
  obdHandle.reset({});
  mConnected = false;
  IVehicleApplication::PublishVehicleEvent(VehicleDisconnected{Clock::Now()});

  INFO("Vehicle Disconnected");
}

auto Vehicle::update() -> VehicleUpdateCode {
  if (!mConnected) { return NONE; }
  auto lock = std::unique_lock { mu };

  if (!mConnected) { return NONE; }
  readPID(PID_RPM, mRpm, NoRemapping);

  if (!mConnected) { return PARTIAL; }
  readPID(PID_COOLANT_TEMP, mCoolantTemp, ToFahrenheit);

  if (!mConnected) { return PARTIAL; }
  readPID(PID_SPEED, mSpeed, NoRemapping);

  if (!mConnected) { return PARTIAL; }
  readPID(PID_THROTTLE, mThrottlePosition, ToPercent);

  if (!mConnected) { return PARTIAL; }
  readPID(PID_FUEL_LEVEL, mFuelLevel, ToPercent);

  return FULL;
}

auto Vehicle::rpm() const -> revs_per_minute {
  return mRpm;
}

auto Vehicle::coolantTemp() const -> fahrenheit {
  return mCoolantTemp;
}

auto Vehicle::fuelLevel() const -> percent {
  return mFuelLevel;
}

auto Vehicle::throttlePosition() const -> percent {
  return mThrottlePosition;
}

auto Vehicle::speed() const-> mph {
  return mSpeed;
}

auto Vehicle::inLowPowerMode() const -> bool {
  return mLowPowerMode;
}

auto Vehicle::isConnected() const -> bool {
  return mConnected;
}

auto Vehicle::setLowPowerMode(bool value) -> void {
  auto lock = std::unique_lock { mu };
  mLowPowerMode = value;
}

}