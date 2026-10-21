//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_CANPROPERTY_H
#define RGBCAR_CANPROPERTY_H

#include "CANMessages.h"
#include "Func.h"

namespace rgb::car {

class Vehicle;
using VehicleSetter = std::function<void(u32, Vehicle&)>;

struct CANParameter {
  CANRequestMessage message{};
  Function<u32(const DataBuffer&)> dataMapper{};
  VehicleSetter vehicleSetter{};
  Duration frequency{Duration::Milliseconds(200)};
  u8 priority{0};
  struct Metrics {
    Timestamp lastRequestedAt{};
    Timestamp lastReceivedAt{};
    Duration averageResponseTime{};
    uint sentMessages{};
    uint droppedMessages{};
    uint sendFailures{};
    u8 failedAttempts{};
  } metrics{};

  auto handles(const ResponseMessage& response, u16 pId) const -> bool;
  auto messageSent(Timestamp at) -> void ;
  [[nodiscard]]
  auto failedAttempt() -> bool ;
  auto responseReceived(const ResponseMessage& response, Timestamp at, Vehicle& vehicle) -> void ;
  void reset(Timestamp initialTime);

  constexpr static auto RESPONSE_TIME_SMOOTHING_FACTOR = 0.2f;

  static auto Rpm(Duration frequency = Duration::Milliseconds(50), u8 priority = 3) -> CANParameter;
  static auto Speed(Duration frequency = Duration::Milliseconds(50), u8 priority = 3) -> CANParameter;
  static auto CoolantTemperature(Duration frequency = Duration::Milliseconds(500), u8 priority = 0) -> CANParameter;
  static auto FuelLevel(Duration frequency = Duration::Milliseconds(500), u8 priority = 0) -> CANParameter;
  static auto ThrottlePosition(Duration frequency = Duration::Milliseconds(50), u8 priority = 2) -> CANParameter;
};

}

#endif //RGBCAR_CANPROPERTY_H
