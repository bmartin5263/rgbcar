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
#include "CANProperty.h"
#include "CANEcu.h"

namespace rgb::car {

class CANBackend : public VehicleBackend {
public:

  constexpr static auto BOTH_BUFFERS = 0x3;
  constexpr static auto BUFFER_0 = 0x1;
  constexpr static auto BUFFER_1 = 0x2;
  constexpr static auto INVALID_BUFFER = 0xFF;
  constexpr static auto LENGTH_BYTE_IDX = 0;
  constexpr static auto MODE_BYTE_IDX = 1;

  enum class ClockRate : u32 {
    MHZ_8 = static_cast<u32>(8E6),
    MHZ_16 = static_cast<u32>(16E6),
    MHZ_25 = static_cast<u32>(25E6),
    MHZ_40 = static_cast<u32>(40E6)
  };

  enum class BaudRate : u32 {
    BAUD_125 = static_cast<u32>(125E3),
    BAUD_250 = static_cast<u32>(250E3),
    BAUD_500 = static_cast<u32>(500E3)
  };

  explicit CANBackend(
    u8 pinNumber,
    ClockRate clockRate,
    tcb::span<CANEcu> ecus,
    tcb::span<CANProperty> properties,
    BaudRate baudRate = BaudRate::BAUD_500,
    bool listenOnly = false);

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto requestPID(CANProperty& property, u8 buffer, Timestamp now) -> bool;
  auto logInformation() const -> void;
  auto resetProperties() -> void;

private:
  auto receive(int buffer, ResponseMessage& message, Vehicle& vehicle) -> bool;
  static auto log(int buffer, const ResponseMessage& message) -> void;
  auto nextProperty() -> CANProperty&;
  auto ecuGate(u16 canId) -> CANEcu&;

  MCP2515 mMcp2515;
  ResponseMessage mBuffer0Message;
  ResponseMessage mBuffer1Message;
  tcb::span<CANEcu> mEcus;
  CANEcu mDefaultEcu{0x7DF};
  tcb::span<CANProperty> mProperties;
  ClockRate mClockRate;
  BaudRate mBaudRate;
  u8 mPinNumber;
  int mNextProperty;
  bool mListenOnly;
  bool mConnected;
};

}

#endif
#endif //RGBLIB_CANMODULE_H
