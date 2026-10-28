//
// Created by Brandon on 7/26/26.
//

#ifndef RGBLIB_CANMODULE_H
#define RGBLIB_CANMODULE_H

#if !RGB_NATIVE

#include <span.hpp>
#include <Log.h>
#include <MCP2515.h>

#include "Types.h"
#include "VehicleBackend.h"
#include "CANParameter.h"
#include "CANEcu.h"
#include "ClockRate.h"
#include "BaudRate.h"

namespace rgb::car {

class CANBackend : public VehicleBackend {
public:

  constexpr static auto BOTH_BUFFERS = 0x3;
  constexpr static auto BUFFER_0 = 0x1;
  constexpr static auto BUFFER_1 = 0x2;
  constexpr static auto INVALID_BUFFER = 0xFF;
  constexpr static auto LENGTH_BYTE_IDX = 0;
  constexpr static auto MODE_BYTE_IDX = 1;

  explicit CANBackend(
    u8 pinNumber,
    ClockRate clockRate,
    BaudRate baudRate,
    tcb::span<CANEcu> ecus,
    tcb::span<CANParameter> properties);

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto logInformation() const -> void;

private:
  static auto log(int buffer, const ResponseMessage& message) -> void;
  auto receive(int buffer, ResponseMessage& message, Vehicle& vehicle) -> bool;
  auto nextParameter() -> CANParameter&;
  auto ecuGate(u16 canId) -> CANEcu&;
  auto requestPID(CANParameter& parameter, u8 buffer, Timestamp now) -> bool;
  auto resetProperties() const -> void;

  MCP2515 mMcp2515;
  ResponseMessage mBuffer0Message;
  ResponseMessage mBuffer1Message;
  tcb::span<CANEcu> mEcus;
  CANEcu mDefaultEcu{0x7DF};
  tcb::span<CANParameter> mProperties;
  ClockRate mClockRate;
  BaudRate mBaudRate;
  u8 mPinNumber;
  int mNextParameter;
  bool mConnected;
};

}

#endif
#endif //RGBLIB_CANMODULE_H
