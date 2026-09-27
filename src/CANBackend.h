//
// Created by Brandon on 7/26/26.
//

#ifndef RGBLIB_CANMODULE_H
#define RGBLIB_CANMODULE_H

#if !RGB_NATIVE

#include <Func.h>
#include <array>
#include <Log.h>

#include <MCP2515.h>

#include "Types.h"
#include "VehicleBackend.h"
#include "Vehicle.h"

namespace rgb::car {

class CANBackend : public VehicleBackend {
public:
  using DataBuffer = u8[8];

  constexpr static auto BOTH_BUFFERS = 0x3;
  constexpr static auto BUFFER_0 = 0x1;
  constexpr static auto BUFFER_1 = 0x2;
  constexpr static auto RESPONSE_TIME_SMOOTHING_FACTOR = 0.2f;
  constexpr static auto INVALID_BUFFER = 0xFF;
  constexpr static auto LENGTH_BYTE_IDX = 0;
  constexpr static auto MODE_BYTE_IDX = 1;


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

  struct RequestMessage {
    u16 canId;            // 11-bit only
    u16 pId;
    u8 mode;

    static constexpr auto ObdII(u8 pId) -> RequestMessage {
      return RequestMessage{
        .canId = 0x7E0, // right now everything is engine ecu
        .pId = pId,
        .mode = 0x1
      };
    }

    static constexpr auto UnifiedDiagnostic(u16 canId, u16 pId) -> RequestMessage {
      return RequestMessage{
        .canId = canId,
        .pId = pId,
        .mode = 0x22
      };
    }
  };

  enum class PropertyType {
    RPM,
    SPEED,
    COOLANT_TEMP,
    FUEL_LEVEL,
    THROTTLE_POSITION,
    GEAR_NUMBER,
    GEAR_POSITION,
    BRAKE_APPLIED,
    OVERDRIVE_ACTIVE,
    TCS_ACTIVE,
    INFO_SWITCH_PRESSED,
    SELECT_SWITCH_PRESSED,

    Count_
  };

  static auto ToString(PropertyType type) -> const char*;

  using VehicleSetter = std::function<void(u32, Vehicle&)>;

  struct Property {
    const PropertyType type{};
    const RequestMessage message{};
    const Function<const DataBuffer&, u32> dataMapper{};
    const VehicleSetter vehicleSetter{};
    const Duration frequency{Duration::Milliseconds(200)};
    const u8 priority{0};
    Timestamp lastRequestedAt{};
    Timestamp lastReceivedAt{};
    Duration averageResponseTime{};
    uint sentMessages{};
    uint droppedMessages{};
    uint sendFailures{};
    u8 failedAttempts{};

    auto handles(const ResponseMessage& response, u16 pId) const -> bool {
      return message.canId == response.requestCANId() && message.pId == pId;
    }

    auto messageSent(Timestamp at) -> void {
      // INFO("%s Message was Sent", ToString(property.type));
      if (lastReceivedAt < lastRequestedAt) {
        // ERROR("%s Message response was Dropped", ToString(type));
        ++droppedMessages;
      }
      lastRequestedAt = at;
      ++sentMessages;
      failedAttempts = 0;
    }

    [[nodiscard]]
    auto failedAttempt() -> bool {
      ++failedAttempts;
      if (failedAttempts >= 3) {
        failedAttempts = 0;
        ++sendFailures;
        return true;
      }
      return false;
    }

    auto responseReceived(const ResponseMessage& response, Timestamp at, Vehicle& vehicle) -> void {
      lastReceivedAt = at;
      auto duration = lastReceivedAt.timeSince(lastRequestedAt);
      averageResponseTime = Duration::Microseconds(RunningAverage(
        averageResponseTime.asMicroseconds(), duration.asMicroseconds(), RESPONSE_TIME_SMOOTHING_FACTOR
      ));
      auto mappedData = dataMapper(response.data);
      vehicleSetter(mappedData, vehicle);
    }
  };

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

  explicit CANBackend(u8 pinNumber, ClockRate clockRate, BaudRate baudRate = BaudRate::BAUD_500, bool listenOnly = false);

  auto connect(Vehicle& vehicle) -> bool override;
  auto isConnected() const -> bool override;
  auto update(Vehicle& vehicle) -> void override;
  auto requestPID(Property& property, u8 buffer, Timestamp now) -> void;
  auto logInformation() const -> void;
  auto resetProperties() -> void;

private:
  auto receive(int buffer, ResponseMessage& message, Vehicle& vehicle) -> bool;
  static auto log(int buffer, const ResponseMessage& message) -> void;
  auto nextProperty() -> Property&;

  MCP2515 mMcp2515;
  ResponseMessage mBuffer0Message;
  ResponseMessage mBuffer1Message;
  std::array<Property, static_cast<int>(PropertyType::Count_) - 1> mProperties = std::array {
    Property{
      .type = PropertyType::RPM,
      .message = RequestMessage::ObdII(0xC),
      .dataMapper = [](const DataBuffer& buffer) {
        auto A = buffer[3];
        auto B = buffer[4];
        return (256 * A + B) / 4;
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setRpm(result);
      },
      .frequency = Duration::Milliseconds(50),
      .priority = 3
    },
    Property{
      .type = PropertyType::SPEED,
      .message = RequestMessage::ObdII(0xD),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[3];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setSpeed(result);
      },
      .frequency = Duration::Milliseconds(50),
      .priority = 3
    },
    Property{
      .type = PropertyType::COOLANT_TEMP,
      .message = RequestMessage::ObdII(0x5),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[3] - 40;
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        vehicle.setCoolantTemp(result);
      },
      .frequency = Duration::Milliseconds(500),
      .priority = 0
    },
    Property{
      .type = PropertyType::FUEL_LEVEL,
      .message = RequestMessage::ObdII(0x2F),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[3];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        auto percent = (100 / 255.f) * result;
        vehicle.setFuelLevel(percent);
      },
      .frequency = Duration::Milliseconds(500),
      .priority = 0
    },
    Property{
      .type = PropertyType::THROTTLE_POSITION,
      .message = RequestMessage::ObdII(0x11),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[3];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        auto percent = (100 / 255.f) * result;
        vehicle.setThrottlePosition(percent);
      },
      .frequency = Duration::Milliseconds(200),
      .priority = 2
    },
    Property{
      .type = PropertyType::GEAR_NUMBER,
      .message = RequestMessage::UnifiedDiagnostic(0x7E0, 0x11B3),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x2:
            vehicle.setGearNumber(1);
            break;
          case 0x4:
            vehicle.setGearNumber(2);
            break;
          case 0x6:
            vehicle.setGearNumber(3);
            break;
          case 0x8:
            vehicle.setGearNumber(4);
            break;
          default:
            ERROR("No Gear Number Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(200),
      .priority = 2
    },
    Property{
      .type = PropertyType::GEAR_POSITION,
      .message = RequestMessage::UnifiedDiagnostic(0x7E0, 0x11B6),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x8C:
            vehicle.setGearPosition(GearPosition::P);
            break;
          case 0x78:
            vehicle.setGearPosition(GearPosition::R);
            break;
          case 0x64:
            vehicle.setGearPosition(GearPosition::N);
            break;
          case 0x58:
          case 0x56:
            vehicle.setGearPosition(GearPosition::D);
            break;
          case 0x2C:
          case 0x2A:
            vehicle.setGearPosition(GearPosition::L);
            break;
          default:
            ERROR("No Gear Position Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(200),
      .priority = 2
    },
    Property{
      .type = PropertyType::BRAKE_APPLIED,
      .message = RequestMessage::UnifiedDiagnostic(0x7E0, 0xA211),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[5];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x8:
            vehicle.setBrakeApplied(false);
            break;
          case 0xA:
            vehicle.setBrakeApplied(true);
            break;
          default:
            ERROR("No Brake Pressure Applied Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(200),
      .priority = 2
    },
    Property{
      .type = PropertyType::OVERDRIVE_ACTIVE,
      .message = RequestMessage::UnifiedDiagnostic(0x7E0, 0x16B5),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0xC0:
            vehicle.setOverdriveActive(false);
            break;
          case 0x80:
            vehicle.setOverdriveActive(true);
            break;
          default:
            ERROR("No Overdrive Active Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(200),
      .priority = 2
    },
    Property{
      .type = PropertyType::TCS_ACTIVE,
      .message = RequestMessage::UnifiedDiagnostic(0x760, 0x2927),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x80:
          case 0x00:
            vehicle.setTCSActive(false);
            break;
          case 0x40:
          case 0xC0:
            vehicle.setTCSActive(true);
            break;
          default:
            ERROR("No TCS Active Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(500),
      .priority = 0
    },
    // Property{
    //   .type = PropertyType::INFO_SWITCH_PRESSED,
    //   .message = RequestMessage::UnifiedDiagnostic(0x720, 0x6101),
    //   .dataMapper = [](const DataBuffer& buffer) {
    //     return buffer[4];
    //   },
    //   .vehicleSetter = [](auto result, auto& vehicle) {
    //     switch (result) {
    //       case 0x0:
    //         vehicle.setInfoButtonPressed(false);
    //         break;
    //       case 0x2:
    //         vehicle.setInfoButtonPressed(true);
    //         break;
    //       default:
    //         ERROR("No Info Button Pressed Mapping For: %x", result);
    //     }
    //   },
    //   .frequency = Duration::Milliseconds(500),
    //   .priority = 0
    // },
    Property{
      .type = PropertyType::SELECT_SWITCH_PRESSED,
      .message = RequestMessage::UnifiedDiagnostic(0x720, 0x610C),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x0:
            vehicle.setSelectButtonPressed(false);
            break;
          case 0x20:
            vehicle.setSelectButtonPressed(true);
            break;
          default:
            ERROR("No Select Button Pressed Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(500),
      .priority = 0
    },
  };
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
