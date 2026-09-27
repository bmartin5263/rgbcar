//
// Created by Brandon on 9/20/26.
//

#if !RGB_NATIVE

#include "ELM327Backend.h"

#include <Clock.h>

#include "IVehicleApplication.h"
#include "Vehicle.h"

namespace rgb::car {

auto ELM327Backend::connect(Vehicle& vehicle) -> bool {
  if (mObdHandle->getState() == OBD_CONNECTED) {
    INFO("Vehicle already connected");
    mConnected = true;
    return true;
  }

  TRACE("Connecting");

  mObdHandle.reset({});
  mConnected = false;

  auto rx = mRx.to<i8>();
  auto tx = mTx.to<i8>();

  if (!mObdHandle->begin(rx, tx)) {
    ERROR("OBDBackend begin() failed with Pins RX=%i, TX=%i", rx, tx);
    return false;
  }

  if (!mObdHandle->init()) {
    ERROR("OBDBackend init() failed with Pins RX=%i, TX=%i", rx, tx);
    return false;
  }

  INFO("OBDBackend ready");
  mConnected = true;
  vehicle.setConnected(true);
  mLastResponse = Clock::Now();

  return true;
}

auto ELM327Backend::isConnected() const -> bool {
  return mConnected;
}

auto ELM327Backend::update(Vehicle& vehicle) -> void {
  if (!mConnected) {
    return;
  }

  for (auto& property : properties) {
    readPID(static_cast<byte>(property.pid), vehicle, property.vehicleSetter);
    if (!mConnected) {
      return;
    }
  }
}

auto ELM327Backend::disconnect(Vehicle& vehicle) -> void {
  mObdHandle.reset({});
  mConnected = false;
  vehicle.setConnected(false);
  INFO("OBDBackend Disconnected");
}

auto ELM327Backend::readPID(byte pid, Vehicle& vehicle, const VehicleSetter& vehicleSetter) -> void {
  auto& obd = *mObdHandle;
  ASSERT(obd.getState() == OBD_CONNECTED, "OBD not connected");

  if (int value; obd.readPID(pid, value, READ_TIMEOUT_MS)) {
    mLastResponse = Clock::Now();
    vehicleSetter(value, vehicle);
  }
  else {
    if (Clock::Now().timeSince(mLastResponse) >= DISCONNECT_TIMEOUT) {
      disconnect(vehicle);
    }
  }
}
}

#endif
