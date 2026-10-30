//
// Created by Brandon on 10/4/26.
//

#ifndef RGBCAR_CANPROPERTY_H
#define RGBCAR_CANPROPERTY_H

#include "CANMessages.h"
#include "Func.h"
#include "Vehicle.h"

namespace rgb::car {

template <typename Vehicle>
using VehicleSetter = std::function<void(u32, Vehicle&)>;

template <typename Vehicle = VehicleImpl>
struct CANParameter {
  CANRequestMessage message{};
  Function<u32(const DataBuffer&)> dataMapper{};
  VehicleSetter<Vehicle> vehicleSetter{};
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


template <typename Vehicle>
auto CANParameter<Vehicle>::Rpm(Duration frequency, u8 priority) -> CANParameter {
  return CANParameter{
    .message = CANRequestMessage::ObdII(0xC),
    .dataMapper = [](const DataBuffer& buffer) {
      auto A = buffer[3];
      auto B = buffer[4];
      return (256 * A + B) / 4;
    },
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setRpm(result);
    },
    .frequency = frequency,
    .priority = priority
  };
}

template <typename Vehicle>
auto CANParameter<Vehicle>::Speed(Duration frequency, u8 priority) -> CANParameter {
  return CANParameter{
    .message = CANRequestMessage::ObdII(0xD),
    .dataMapper = [](const DataBuffer& buffer) {
      return buffer[3];
    },
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setSpeed(result);
    },
    .frequency = frequency,
    .priority = priority
  };
}

template <typename Vehicle>
auto CANParameter<Vehicle>::CoolantTemperature(Duration frequency, u8 priority) -> CANParameter {
  return CANParameter{
    .message = CANRequestMessage::ObdII(0x5),
    .dataMapper = [](const DataBuffer& buffer) {
      return buffer[3] - 40;
    },
    .vehicleSetter = [](auto result, auto& vehicle) {
      vehicle.setCoolantTemp(result);
    },
    .frequency = frequency,
    .priority = priority
  };
}

template <typename Vehicle>
auto CANParameter<Vehicle>::FuelLevel(Duration frequency, u8 priority) -> CANParameter {
  return CANParameter{
    .message = CANRequestMessage::ObdII(0x2F),
    .dataMapper = [](const DataBuffer& buffer) {
      return buffer[3];
    },
    .vehicleSetter = [](auto result, auto& vehicle) {
      auto percent = (100 / 255.f) * result;
      vehicle.setFuelLevel(percent);
    },
    .frequency = frequency,
    .priority = priority
  };
}

template <typename Vehicle>
auto CANParameter<Vehicle>::ThrottlePosition(Duration frequency, u8 priority) -> CANParameter {
  return CANParameter{
    .message = CANRequestMessage::ObdII(0x11),
    .dataMapper = [](const DataBuffer& buffer) {
      return buffer[3];
    },
    .vehicleSetter = [](auto result, auto& vehicle) {
      auto percent = (100 / 255.f) * result;
      vehicle.setThrottlePosition(percent);
    },
    .frequency = frequency,
    .priority = priority
  };
}

template <typename Vehicle>
auto CANParameter<Vehicle>::handles(const ResponseMessage& response, u16 pId) const -> bool {
  return message.canId == response.requestCANId() && message.pId == pId;
}

template <typename Vehicle>
auto CANParameter<Vehicle>::messageSent(Timestamp at) -> void {
  // INFO("%s Message was Sent", ToString(parameter.type));
  if (metrics.lastReceivedAt < metrics.lastRequestedAt) {
    // ERROR("%s Message response was Dropped", ToString(type));
    ++metrics.droppedMessages;
  }
  metrics.lastRequestedAt = at;
  ++metrics.sentMessages;
  metrics.failedAttempts = 0;
}

template <typename Vehicle>
auto CANParameter<Vehicle>::failedAttempt() -> bool {
  ++metrics.failedAttempts;
  if (metrics.failedAttempts >= 3) {
    metrics.failedAttempts = 0;
    ++metrics.sendFailures;
    return true;
  }
  return false;
}

template <typename Vehicle>
auto CANParameter<Vehicle>::responseReceived(const ResponseMessage& response, Timestamp at, Vehicle& vehicle) -> void {
  metrics.lastReceivedAt = at;
  auto duration = metrics.lastReceivedAt.timeSince(metrics.lastRequestedAt);
  metrics.averageResponseTime = Duration::Microseconds(RunningAverage(
    metrics.averageResponseTime.asMicroseconds(), duration.asMicroseconds(), RESPONSE_TIME_SMOOTHING_FACTOR
  ));
  auto mappedData = dataMapper(response.data);
  vehicleSetter(mappedData, vehicle);
}

template <typename Vehicle>
void CANParameter<Vehicle>::reset(Timestamp initialTime) {
  metrics.lastRequestedAt = initialTime;
  metrics.lastReceivedAt = initialTime;
  metrics.averageResponseTime = Duration::Zero();
  metrics.sentMessages = 0;
  metrics.droppedMessages = 0;
  metrics.sendFailures = 0;
}

}

#endif //RGBCAR_CANPROPERTY_H
