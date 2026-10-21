//
// Created by Brandon on 10/4/26.
//

#include "CANParameter.h"
#include "Vehicle.h"

namespace rgb::car {

auto CANParameter::Rpm(Duration frequency, u8 priority) -> CANParameter {
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

auto CANParameter::Speed(Duration frequency, u8 priority) -> CANParameter {
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

auto CANParameter::CoolantTemperature(Duration frequency, u8 priority) -> CANParameter {
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

auto CANParameter::FuelLevel(Duration frequency, u8 priority) -> CANParameter {
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

auto CANParameter::ThrottlePosition(Duration frequency, u8 priority) -> CANParameter {
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

auto CANParameter::handles(const ResponseMessage& response, u16 pId) const -> bool {
  return message.canId == response.requestCANId() && message.pId == pId;
}

auto CANParameter::messageSent(Timestamp at) -> void {
  // INFO("%s Message was Sent", ToString(parameter.type));
  if (metrics.lastReceivedAt < metrics.lastRequestedAt) {
    // ERROR("%s Message response was Dropped", ToString(type));
    ++metrics.droppedMessages;
  }
  metrics.lastRequestedAt = at;
  ++metrics.sentMessages;
  metrics.failedAttempts = 0;
}

auto CANParameter::failedAttempt() -> bool {
  ++metrics.failedAttempts;
  if (metrics.failedAttempts >= 3) {
    metrics.failedAttempts = 0;
    ++metrics.sendFailures;
    return true;
  }
  return false;
}

auto CANParameter::responseReceived(const ResponseMessage& response, Timestamp at, Vehicle& vehicle) -> void {
  metrics.lastReceivedAt = at;
  auto duration = metrics.lastReceivedAt.timeSince(metrics.lastRequestedAt);
  metrics.averageResponseTime = Duration::Microseconds(RunningAverage(
    metrics.averageResponseTime.asMicroseconds(), duration.asMicroseconds(), RESPONSE_TIME_SMOOTHING_FACTOR
  ));
  auto mappedData = dataMapper(response.data);
  vehicleSetter(mappedData, vehicle);
}

void CANParameter::reset(Timestamp initialTime) {
  metrics.lastRequestedAt = initialTime;
  metrics.lastReceivedAt = initialTime;
  metrics.averageResponseTime = Duration::Zero();
  metrics.sentMessages = 0;
  metrics.droppedMessages = 0;
  metrics.sendFailures = 0;
}
}
