//
// Created by Brandon on 9/20/26.
//

#ifndef RGBCAR_OBDBACKEND_H
#define RGBCAR_OBDBACKEND_H

#if !RGB_NATIVE

#include <Handle.h>
#include <span.hpp>
#include <Pin.h>

#include "ELMParameter.h"
#include "OBD.h"
#include "VehicleBackend.h"

namespace rgb::car {

class ELMBackend : public VehicleBackend {
public:
  static constexpr auto DISCONNECT_TIMEOUT = Duration::Milliseconds(100);
  static constexpr auto READ_TIMEOUT_MS = 25;

  ELMBackend(PinNumber rx, PinNumber tx, tcb::span<ELMParameter> properties)
    : mObdHandle{{}}, mProperties(properties), mLastResponse{}, mLastUpdate{}, mRx(rx), mTx(tx), mConnected(false) {
  }

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto disconnect(Vehicle& vehicle) -> void;

private:
  Handle<COBD, OBDDestroyer> mObdHandle;
  tcb::span<ELMParameter> mProperties;
  Timestamp mLastResponse;
  Timestamp mLastUpdate;
  PinNumber mRx;
  PinNumber mTx;
  bool mConnected;

  auto readPID(byte pid, Vehicle& vehicle, const VehicleSetter& vehicleSetter) -> bool;
};

}

#endif //RGBCAR_OBDBACKEND_H
#endif