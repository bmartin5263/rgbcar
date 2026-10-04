//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_CANECU_H
#define RGBCAR_CANECU_H

#include "Types.h"

namespace rgb::car {
struct CANEcu {
  constexpr static auto DEFAULT_ECU_RESPONSE_TIMEOUT = Duration::Milliseconds(50);

  u16 canId{};
  Duration timeout{};
  Timestamp requestedAt{};
  bool awaitingResponse{false};

  explicit CANEcu(u16 can_id, Duration timeout = DEFAULT_ECU_RESPONSE_TIMEOUT)
    : canId(can_id),
      timeout(timeout) {
  }

  [[nodiscard]] auto isAvailable(Timestamp now) const -> bool
  {
    return !awaitingResponse || now.timeSince(requestedAt) >= timeout;
  }

  void reset() {
    requestedAt = Timestamp::Zero();
    awaitingResponse = false;
  }
};
}

#endif //RGBCAR_CANECU_H
