//
// Created by Brandon on 10/11/25.
//

#ifndef RGBCAR_CANAPPLICATION_H
#define RGBCAR_CANAPPLICATION_H

#include <Clock.h>
#include <Log.h>

#include "CANBackend.h"
#include "Every.h"
#include "IVehicleApplication.h"
#include "VehicleApplication.h"

using namespace rgb;
using namespace rgb::car;

class MyVehicle : public VehicleImpl {
public:
  auto gearNumber() const -> u8 { return mGearNumber; }
  auto gearPosition() const -> GearPosition { return mGearPosition; }
  auto isBrakeApplied() const -> bool { return mBrakeApplied; }
  auto isOverdriveActive() const -> bool { return mOverdriveActive; }
  auto isTCSActive() const -> bool { return mTCSActive; }
  auto isInfoButtonPressed() const -> bool { return mInfoButtonPressed; }
  auto isSelectButtonPressed() const -> bool { return mSelectButtonPressed; }
  auto areParkingLightsActive() const -> bool { return mParkingLightsActive; }

  auto setGearNumber(u8 value) -> void {
    if (auto prev = mGearNumber.exchange(value); prev != value) {
      IVehicleApplication::PublishVehicleEvent(GearNumberChanged{{ Clock::Now() }});
    }
  }

  auto setGearPosition(GearPosition value) -> void {
    if (auto prev = mGearPosition.exchange(value); prev != value) {
      INFO("Gear Position Changed");
      IVehicleApplication::PublishVehicleEvent(GearPositionChanged{{ Clock::Now() }});
    }
  }

  auto setBrakeApplied(bool value) -> void {
    if (auto prev = mBrakeApplied.exchange(value); prev != value) {
      if (value) {
        INFO("Brake Pressed");
        IVehicleApplication::PublishVehicleEvent(BrakePressed{{ Clock::Now() }});
      }
      else {
        INFO("Brake Released");
        IVehicleApplication::PublishVehicleEvent(BrakeReleased{{ Clock::Now() }});
      }
    }
  }

  auto setOverdriveActive(bool value) -> void {
    if (auto prev = mOverdriveActive.exchange(value); prev != value) {
      if (value) {
        INFO("Overdrive Activated");
        IVehicleApplication::PublishVehicleEvent(OverdriveActivated{{ Clock::Now() }});
      }
      else {
        INFO("Overdrive Deactivated");
        IVehicleApplication::PublishVehicleEvent(OverdriveDeactivated{{ Clock::Now() }});
      }
    }
  }

  auto setTCSActive(bool value) -> void {
    if (auto prev = mTCSActive.exchange(value); prev != value) {
      if (value) {
        INFO("TCS Activated");
        IVehicleApplication::PublishVehicleEvent(TractionControlActivated{{ Clock::Now() }});
      }
      else {
        INFO("TCS Deactivated");
        IVehicleApplication::PublishVehicleEvent(TractionControlDeactivated{{ Clock::Now() }});
      }
    }
  }

  auto setInfoButtonPressed(bool value) -> void {
    if (auto prev = mInfoButtonPressed.exchange(value); prev != value && value) {
      INFO("Info Pressed");
      IVehicleApplication::PublishVehicleEvent(InfoButtonPressed{{ Clock::Now() }});
    }
  }

  auto setSelectButtonPressed(bool value) -> void {
    if (auto prev = mSelectButtonPressed.exchange(value); prev != value && value) {
      INFO("Select Pressed");
      IVehicleApplication::PublishVehicleEvent(SelectButtonPressed{{ Clock::Now() }});
    }
  }

  auto setParkingLightsActive(bool value) -> void {
    if (auto prev = mParkingLightsActive.exchange(value); prev != value) {
      if (value) {
        INFO("Parking Lights Activated");
        IVehicleApplication::PublishVehicleEvent(ParkingLightsActivated{{ Clock::Now() }});
      }
      else {
        INFO("Parking Lights Deactivated");
        IVehicleApplication::PublishVehicleEvent(ParkingLightsDeactivated{{ Clock::Now() }});
      }
    }
  }

private:
  atomic<u8> mGearNumber{};
  atomic<GearPosition> mGearPosition{};
  atomic<bool> mBrakeApplied{};
  atomic<bool> mOverdriveActive{true};
  atomic<bool> mTCSActive{true};
  atomic<bool> mInfoButtonPressed{};
  atomic<bool> mSelectButtonPressed{};
  atomic<bool> mParkingLightsActive{};
};

class CANApplication : public VehicleApplication<VehicleEvents, MyVehicle> {
protected:
  auto configure(Configurer& app) -> void override {
  }

#if !RGB_NATIVE
  auto vehicleBackend() -> VehicleBackend<MyVehicle>& override {
    static auto ecus = std::array {
      CANEcu{0x7DF}, CANEcu{0x760}, CANEcu{0x720}
    };
    static auto parameters = std::array{
      CANParameter<MyVehicle>::Rpm(),
      CANParameter<MyVehicle>::Speed(),
      CANParameter<MyVehicle>::CoolantTemperature(),
      CANParameter<MyVehicle>::FuelLevel(),
      CANParameter<MyVehicle>::ThrottlePosition(),
      CANParameter<MyVehicle>{
        .message = CANRequestMessage::UnifiedDiagnostic(0x7DF, 0x11B3),
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
      CANParameter<MyVehicle>{
        .message = CANRequestMessage::UnifiedDiagnostic(0x7DF, 0x11B6),
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
      CANParameter<MyVehicle>{
        .message = CANRequestMessage::UnifiedDiagnostic(0x7DF, 0xA211),
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
      CANParameter<MyVehicle>{
        .message = CANRequestMessage::UnifiedDiagnostic(0x7DF, 0x16B5),
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
      CANParameter<MyVehicle>{
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
      CANParameter<MyVehicle>{
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
      CANParameter<MyVehicle>{
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
    static auto backend = CANBackend<MyVehicle>{PinNumber{D11}, ClockRate::MHZ_8, BaudRate::BAUD_500, ecus, parameters};
    return backend;
  }
#endif

  auto update() -> void override {
    if (static auto lastLoggedDataAt = Timestamp{}; every(Duration::Seconds(1), lastLoggedDataAt)) {
#if !RGB_NATIVE
      static_cast<CANBackend<MyVehicle>&>(vehicleBackend()).logInformation();
#endif
      vehicle.logInformation();
    }
  }
};


#endif //RGBCAR_CANAPPLICATION_H
