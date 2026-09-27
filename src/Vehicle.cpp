//
// Created by Brandon on 9/20/26.
//

#include "Vehicle.h"

#include <Clock.h>
#include <Log.h>

#include "IVehicleApplication.h"

namespace rgb::car {

auto Vehicle::rpm() const -> revs_per_minute {
  return mRpm;
}

auto Vehicle::speed() const -> kph {
  return mSpeed;
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

auto Vehicle::gearNumber() const -> u8 {
  return mGearNumber;
}

auto Vehicle::gearPosition() const -> GearPosition {
  return mGearPosition;
}

auto Vehicle::isBrakeApplied() const -> bool {
  return mBrakeApplied;
}

auto Vehicle::isOverdriveActive() const -> bool {
  return mOverdriveActive;
}

auto Vehicle::isTCSActive() const -> bool {
  return mTCSActive;
}

auto Vehicle::isInfoButtonPressed() const -> bool {
  return mInfoButtonPressed;
}

auto Vehicle::isSelectButtonPressed() const -> bool {
  return mSelectButtonPressed;
}

auto Vehicle::isConnected() const -> bool {
  return mConnected;
}

auto Vehicle::logInformation() const -> void {
  static constexpr auto BoolToString = [](bool value) -> const char* { return value ? "true" : "false"; };

  log::printMessage("%-22s | %s", "Property", "Value");
  log::printMessage("-------------------------------------");
  log::printMessage("%-22s | %d rpm", "RPM", rpm());
  log::printMessage("%-22s | %d mph", "Speed", ToMph(speed()));
  log::printMessage("%-22s | %.1f F", "Coolant Temp", coolantTemp());
  log::printMessage("%-22s | %.1f%%", "Fuel Level", fuelLevel());
  log::printMessage("%-22s | %.1f%%", "Throttle Position", throttlePosition());
  log::printMessage("%-22s | %d", "Gear Number", gearNumber());
  log::printMessage("%-22s | %s", "Gear Position", ToString(gearPosition()));
  log::printMessage("%-22s | %s", "Brake Applied", BoolToString(isBrakeApplied()));
  log::printMessage("%-22s | %s", "Overdrive Active", BoolToString(isOverdriveActive()));
  log::printMessage("%-22s | %s", "TCS Active", BoolToString(isTCSActive()));
  log::printMessage("%-22s | %s", "Info Button Pressed", BoolToString(isInfoButtonPressed()));
  log::printMessage("%-22s | %s", "Select Button Pressed", BoolToString(isSelectButtonPressed()));
  log::printMessage("%-22s | %s", "Connected", BoolToString(isConnected()));
}

auto Vehicle::setRpm(revs_per_minute value) -> void {
  mRpm = value;
}

auto Vehicle::setSpeed(kph value) -> void {
  mSpeed = value;
}

auto Vehicle::setCoolantTemp(fahrenheit value) -> void {
  mCoolantTemp = value;
}

auto Vehicle::setFuelLevel(percent value) -> void {
  mFuelLevel = value;
}

auto Vehicle::setThrottlePosition(percent value) -> void {
  mThrottlePosition = value;
}

auto Vehicle::setGearNumber(u8 value) -> void {
  if (auto prev = mGearNumber.exchange(value); prev != value) {
    IVehicleApplication::PublishVehicleEvent(GearNumberChanged{{ Clock::Now() }, prev, value});
  }
}

auto Vehicle::setGearPosition(GearPosition value) -> void {
  if (auto prev = mGearPosition.exchange(value); prev != value) {
    INFO("Gear Position Changed");
    IVehicleApplication::PublishVehicleEvent(GearPositionChanged{{ Clock::Now() }, prev, value});
  }
}

auto Vehicle::setBrakeApplied(bool value) -> void {
  if (auto prev = mBrakeApplied.exchange(value); prev != value) {
    if (value) {
      INFO("Brake Pressed");
      IVehicleApplication::PublishVehicleEvent(BrakePressed{{Clock::Now()}});
    }
    else {
      INFO("Brake Released");
      IVehicleApplication::PublishVehicleEvent(BrakeReleased{{Clock::Now()}});
    }
  }
}

auto Vehicle::setOverdriveActive(bool value) -> void {
  if (auto prev = mOverdriveActive.exchange(value); prev != value) {
    if (value) {
      INFO("Overdrive Activated");
      IVehicleApplication::PublishVehicleEvent(OverdriveActivated{{Clock::Now()}});
    }
    else {
      INFO("Overdrive Deactivated");
      IVehicleApplication::PublishVehicleEvent(OverdriveDeactivated{{Clock::Now()}});
    }
  }
}

auto Vehicle::setTCSActive(bool value) -> void {
  if (auto prev = mTCSActive.exchange(value); prev != value) {
    if (value) {
      INFO("TCS Activated");
      IVehicleApplication::PublishVehicleEvent(TractionControlActivated{{Clock::Now()}});
    }
    else {
      INFO("TCS Deactivated");
      IVehicleApplication::PublishVehicleEvent(TractionControlDeactivated{{Clock::Now()}});
    }
  }
}

auto Vehicle::setInfoButtonPressed(bool value) -> void {
  if (auto prev = mInfoButtonPressed.exchange(value); prev != value) {
    if (value) {
      INFO("Info Pressed");
      IVehicleApplication::PublishVehicleEvent(InfoButtonPressed{{Clock::Now()}});
    }
  }
}

auto Vehicle::setSelectButtonPressed(bool value) -> void {
  if (auto prev = mSelectButtonPressed.exchange(value); prev != value) {
    if (value) {
      INFO("Select Pressed");
      IVehicleApplication::PublishVehicleEvent(SelectButtonPressed{{Clock::Now()}});
    }
  }
}

auto Vehicle::setConnected(bool value) -> void {
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
