//
// Created by Brandon on 10/4/26.
//

#include "CANProperty.h"
#include "Vehicle.h"

namespace rgb::car {

static auto RpmVehicleSetter(u32 result, Vehicle& vehicle) -> void {
  vehicle.setRpm(result);
}

auto ToString(PropertyType type) -> const char* {
  switch (type) {
    case PropertyType::RPM: return "RPM";
    case PropertyType::SPEED: return "SPEED";
    case PropertyType::COOLANT_TEMP: return "COOLANT_TEMP";
    case PropertyType::FUEL_LEVEL: return "FUEL_LEVEL";
    case PropertyType::THROTTLE_POSITION: return "THROTTLE_POSITION";
    case PropertyType::GEAR_NUMBER: return "GEAR_NUMBER";
    case PropertyType::GEAR_POSITION: return "GEAR_POSITION";
    case PropertyType::BRAKE_APPLIED: return "BRAKE_APPLIED";
    case PropertyType::OVERDRIVE_ACTIVE: return "OVERDRIVE_ACTIVE";
    case PropertyType::TCS_ACTIVE: return "TCS_ACTIVE";
    case PropertyType::INFO_SWITCH_PRESSED: return "INFO_SWITCH_PRESSED";
    case PropertyType::SELECT_SWITCH_PRESSED: return "SELECT_SWITCH_PRESSED";
    case PropertyType::Count_: return "UNKNOWN";
  }
  return "UNKNOWN";
}

auto CANProperty::Rpm(Duration frequency, u8 priority) -> CANProperty {
  return CANProperty{
    .type = PropertyType::RPM,
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

auto CANProperty::Speed(Duration frequency, u8 priority) -> CANProperty {
  return CANProperty{
    .type = PropertyType::SPEED,
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

auto CANProperty::CoolantTemperature(Duration frequency, u8 priority) -> CANProperty {
  return CANProperty{
    .type = PropertyType::COOLANT_TEMP,
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

auto CANProperty::FuelLevel(Duration frequency, u8 priority) -> CANProperty {
  return     CANProperty{
    .type = PropertyType::FUEL_LEVEL,
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

auto CANProperty::ThrottlePosition(Duration frequency, u8 priority) -> CANProperty {
  return CANProperty{
    .type = PropertyType::THROTTLE_POSITION,
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

auto CANProperty::handles(const ResponseMessage& response, u16 pId) const -> bool {
  return message.canId == response.requestCANId() && message.pId == pId;
}

auto CANProperty::messageSent(Timestamp at) -> void {
  // INFO("%s Message was Sent", ToString(property.type));
  if (metrics.lastReceivedAt < metrics.lastRequestedAt) {
    // ERROR("%s Message response was Dropped", ToString(type));
    ++metrics.droppedMessages;
  }
  metrics.lastRequestedAt = at;
  ++metrics.sentMessages;
  metrics.failedAttempts = 0;
}

auto CANProperty::failedAttempt() -> bool {
  ++metrics.failedAttempts;
  if (metrics.failedAttempts >= 3) {
    metrics.failedAttempts = 0;
    ++metrics.sendFailures;
    return true;
  }
  return false;
}

auto CANProperty::responseReceived(const ResponseMessage& response, Timestamp at, Vehicle& vehicle) -> void {
  metrics.lastReceivedAt = at;
  auto duration = metrics.lastReceivedAt.timeSince(metrics.lastRequestedAt);
  metrics.averageResponseTime = Duration::Microseconds(RunningAverage(
    metrics.averageResponseTime.asMicroseconds(), duration.asMicroseconds(), RESPONSE_TIME_SMOOTHING_FACTOR
  ));
  auto mappedData = dataMapper(response.data);
  vehicleSetter(mappedData, vehicle);
}

void CANProperty::reset(Timestamp initialTime) {
  metrics.lastRequestedAt = initialTime;
  metrics.lastReceivedAt = initialTime;
  metrics.averageResponseTime = Duration::Zero();
  metrics.sentMessages = 0;
  metrics.droppedMessages = 0;
  metrics.sendFailures = 0;
}
}
