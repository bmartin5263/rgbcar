//
// Created by Brandon on 10/11/25.
//

#ifndef RGBCAR_SNIFFAPPLICATION_H
#define RGBCAR_SNIFFAPPLICATION_H

#include "CANBackend.h"
#include "Every.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

#if RGB_NATIVE
#include "MockBackend.h"
inline auto& backend = MockBackend::Instance();
#else
#include "CANBackend.h"
inline auto ecus = std::array {
    CANEcu{0x7E0}, CANEcu{0x760}, CANEcu{0x720}
};
inline auto properties = std::array {
    CANProperty::Rpm(),
    CANProperty::Speed(),
    CANProperty::CoolantTemperature(),
    CANProperty::FuelLevel(),
    CANProperty::ThrottlePosition(),
    CANProperty{
      .type = PropertyType::GEAR_NUMBER,
      .message = CANRequestMessage::UnifiedDiagnostic(0x7E0, 0x11B3),
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
      .frequency = Duration::Milliseconds(50),
      .priority = 2
    },
    CANProperty{
      .type = PropertyType::GEAR_POSITION,
      .message = CANRequestMessage::UnifiedDiagnostic(0x7E0, 0x11B6),
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
      .frequency = Duration::Milliseconds(50),
      .priority = 2
    },
    CANProperty{
      .type = PropertyType::BRAKE_APPLIED,
      .message = CANRequestMessage::UnifiedDiagnostic(0x7E0, 0xA211),
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
      .frequency = Duration::Milliseconds(50),
      .priority = 2
    },
    CANProperty{
      .type = PropertyType::OVERDRIVE_ACTIVE,
      .message = CANRequestMessage::UnifiedDiagnostic(0x7E0, 0x16B5),
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
      .frequency = Duration::Milliseconds(50),
      .priority = 2
    },
    CANProperty{
      .type = PropertyType::TCS_ACTIVE,
      .message = CANRequestMessage::UnifiedDiagnostic(0x760, 0x2927),
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
    CANProperty{
      .type = PropertyType::INFO_SWITCH_PRESSED,
      .message = CANRequestMessage::UnifiedDiagnostic(0x720, 0x6101),
      .dataMapper = [](const DataBuffer& buffer) {
        return buffer[4];
      },
      .vehicleSetter = [](auto result, auto& vehicle) {
        switch (result) {
          case 0x0:
            vehicle.setInfoButtonPressed(false);
            break;
          case 0x2:
            vehicle.setInfoButtonPressed(true);
            break;
          default:
            ERROR("No Info Button Pressed Mapping For: %x", result);
        }
      },
      .frequency = Duration::Milliseconds(100),
      .priority = 0
    },
    CANProperty{
      .type = PropertyType::SELECT_SWITCH_PRESSED,
      .message = CANRequestMessage::UnifiedDiagnostic(0x720, 0x610C),
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
      .frequency = Duration::Milliseconds(100),
      .priority = 0
    },
  };
inline auto backend = CANBackend{A0, CANBackend::ClockRate::MHZ_8, ecus, properties};
#endif

class SniffApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override {

  }

  auto vehicleBackend() -> VehicleBackend& override {
    return backend;
  }

  auto update() -> void override {
    auto& vehicle = Vehicle::Instance();
    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
      backend.logInformation();
      vehicle.logInformation();
    }
  }
};


#endif //RGBCAR_SNIFFAPPLICATION_H
