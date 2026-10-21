//
// Created by Brandon on 9/20/26.
//

#if !RGB_NATIVE

#include "ELMBackend.h"
#include <Clock.h>
#include <Assertions.h>
#include "Vehicle.h"

namespace rgb::car {

auto ELMBackend::connect(Vehicle& vehicle) -> bool {
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
    ERROR("ELMBackend begin() failed with Pins RX=%i, TX=%i", rx, tx);
    return false;
  }

  if (!mObdHandle->init()) {
    ERROR("ELMBackend init() failed with Pins RX=%i, TX=%i", rx, tx);
    return false;
  }

  INFO("ELMBackend ready");
  mConnected = true;
  vehicle.setConnected(true);
  mLastResponse = Clock::Now();

  return true;
}

auto ELMBackend::isConnected() const -> bool {
  return mConnected;
}

auto ELMBackend::update(Vehicle& vehicle) -> void {
  if (!mConnected) {
    return;
  }

  for (auto& [pid, vehicleSetter] : mProperties) {
    if (!readPID(pid, vehicle, vehicleSetter)) {
      if (Clock::Now().timeSince(mLastResponse) >= DISCONNECT_TIMEOUT) {
        disconnect(vehicle);
        return;
      }
    }
  }
}

auto ELMBackend::disconnect(Vehicle& vehicle) -> void {
  mObdHandle.reset({});
  mConnected = false;
  vehicle.setConnected(false);
  INFO("ELMBackend Disconnected");
}

auto ELMBackend::readPID(byte pid, Vehicle& vehicle, const VehicleSetter& vehicleSetter) -> bool {
  auto& obd = *mObdHandle;
  ASSERT(obd.getState() == OBD_CONNECTED, "OBD not connected");

  if (int value; obd.readPID(pid, value, READ_TIMEOUT_MS)) {
    mLastResponse = Clock::Now();
    vehicleSetter(value, vehicle);
    return true;
  }
  return false;
}
}

#endif
