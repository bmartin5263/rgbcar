//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_CANMESSAGES_H
#define RGBCAR_CANMESSAGES_H

#include <Types.h>

namespace rgb::car {

using DataBuffer = u8[8];

struct ResponseMessage {
  u32 canId{};          // 11-bit standard ID or a 29-bit extended ID
  u8 dlc{};             // Data length code, 0-8
  DataBuffer data{};    // Actual data
  bool rtr{};           // Remote Transmission Request flag - true if this is a request for data with no payload rather than a data frame
  bool extended{};      // Is this an extended frame?

  auto requestCANId() const -> u32 {
    return canId - 8;
  }
};

struct CANRequestMessage {
  u16 canId{};            // 11-bit only
  u16 pId{};
  u8 mode{};

  static constexpr auto ObdII(u8 pId) -> CANRequestMessage {
    return CANRequestMessage{
      .canId = 0x7E0, // right now everything is engine ecu
      .pId = pId,
      .mode = 0x1
    };
  }

  static constexpr auto UnifiedDiagnostic(u16 canId, u16 pId) -> CANRequestMessage {
    return CANRequestMessage{
      .canId = canId,
      .pId = pId,
      .mode = 0x22
    };
  }
};


}

#endif //RGBCAR_CANMESSAGES_H
