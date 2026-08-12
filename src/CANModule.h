//
// Created by Brandon on 7/26/26.
//

#ifndef RGBLIB_CANMODULE_H
#define RGBLIB_CANMODULE_H

#include <MCP2515.h>

#include "Types.h"

namespace rgb::car {

class CANModule {
public:
  constexpr static auto BOTH_BUFFERS = 0x3;
  constexpr static auto BUFFER_1 = 0x1;
  constexpr static auto BUFFER_2 = 0x2;

  struct MCP2515Message {
    u32 id{};             // 11-bit standard ID or a 29-bit extended ID
    u8 dlc{};             // Data length code, 0-8
    u8 data[8]{};         // Actual data
    bool rtr{};           // Remote Transmission Request flag - true if this is a request for data with no payload rather than a data frame
    bool extended{};      // Is this an extended frame?
  };

  enum class ClockRate : u32 {
    MHZ_8 = static_cast<u32>(8E6),
    MHZ_16 = static_cast<u32>(16E6),
    MHZ_25 = static_cast<u32>(25E6),
    MHZ_40 = static_cast<u32>(40E6)
  };

  enum class BaudRate : u64 {
    BAUD_250 = static_cast<u32>(250E3),
    BAUD_500 = static_cast<u32>(500E3)
  };

  explicit CANModule(u8 pinNumber, ClockRate clockRate);

  auto connect(BaudRate baudRate = BaudRate::BAUD_500, bool listenOnly = true) -> bool;
  auto isConnected() const -> bool;
  auto update() -> void;
  auto request() -> void;

private:
  auto receive(int buffer, MCP2515Message& message) -> bool;
  static auto log(int buffer, const MCP2515Message& message) -> void;

  MCP2515 mcp2515{};
  bool mConnected{false};
};

}

#endif //RGBLIB_CANMODULE_H
