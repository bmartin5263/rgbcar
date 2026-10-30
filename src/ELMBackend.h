//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_OBDBACKEND_H
#define RGBCAR_OBDBACKEND_H

#if !RGB_NATIVE

#include <Handle.h>
#include <span.hpp>
#include <Pin.h>
#include <Clock.h>
#include <Assertions.h>

#include "ELMParameter.h"
#include "OBD.h"
#include "VehicleBackend.h"

namespace rgb::car {

template <typename Vehicle = VehicleImpl>
class ELMBackend : public VehicleBackend<Vehicle> {
public:
  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);
  static constexpr auto READ_TIMEOUT_MS = 25;

  ELMBackend(PinNumber rx, PinNumber tx, tcb::span<ELMParameter<Vehicle>> properties)
    : mObdHandle{{}}, mProperties(properties), mLastResponse{}, mLastUpdate{}, mRx(rx), mTx(tx), mConnected(false) {
  }

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto disconnect(Vehicle& vehicle) -> void;

private:
  Handle<COBD, OBDDestroyer> mObdHandle;
  tcb::span<ELMParameter<Vehicle>> mProperties;
  Timestamp mLastResponse;
  Timestamp mLastUpdate;
  PinNumber mRx;
  PinNumber mTx;
  bool mConnected;

  auto readPID(byte pid, Vehicle& vehicle, const VehicleSetter<Vehicle>& vehicleSetter) -> bool;
};


template <typename Vehicle>
auto ELMBackend<Vehicle>::connect(Vehicle& vehicle) -> bool {
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

template <typename Vehicle>
auto ELMBackend<Vehicle>::isConnected() const -> bool {
  return mConnected;
}

template <typename Vehicle>
auto ELMBackend<Vehicle>::update(Vehicle& vehicle) -> void {
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

template <typename Vehicle>
auto ELMBackend<Vehicle>::disconnect(Vehicle& vehicle) -> void {
  mObdHandle.reset({});
  mConnected = false;
  vehicle.setConnected(false);
  INFO("ELMBackend Disconnected");
}

template <typename Vehicle>
auto ELMBackend<Vehicle>::readPID(byte pid, Vehicle& vehicle, const VehicleSetter<Vehicle>& vehicleSetter) -> bool {
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

#endif //RGBCAR_OBDBACKEND_H
#endif