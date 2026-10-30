# rgbcar
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bdon/library/rgbcar.svg)](https://registry.platformio.org/libraries/bdon/rgbcar)

Vehicle-based extension to [rgblib](https://github.com/bmartin5263/rgblib) adding vehicle connectivity. See it in action on [Instagram](https://rgbcar.io)

Features:
- Vehicle reading pinned to ESP32's second core to prevent interfering with LED effects rendering
- ELM327 and CAN backends that canbe customized based on which PIDs you are interested in
- Mock vehicle interface for local testing

## Example

```c++
#include "VehicleApplication.h"
#include "CANBackend.h"
#include "LEDStrip.h"

using namespace rgb;
using namespace rgb::car;

#if RGB_NATIVE
#include "MockBackend.h"
inline auto& backend = MockBackend::Instance();
#else
#include "CANBackend.h"
inline auto ecus = std::array {
    CANEcu{0x7E0}
};
inline auto parameters = std::array {
    CANParameter::Rpm(),
    CANParameter::Speed(),
    CANParameter::CoolantTemperature(),
    CANParameter::FuelLevel(),
    CANParameter::ThrottlePosition(),
  };
inline auto backend = CANBackend{D11, ClockRate::MHZ_8, BaudRate::BAUD_500, ecus, parameters};
#endif

inline auto ledStrip = LEDStrip<12, D2>{};

class ExampleApplication : public VehicleApplication<> {
protected:
  auto configure(Configurer& app) -> void override { app.addPixels(ledStrip); }
  auto vehicleBackend() -> VehicleBackend& override { return backend; }

  auto update() -> void override {
    ledStrip.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 5000.f));
  }
};
```

## Vehicle Template

`VehicleApplication<>` takes in as its 2nd parameter the `Vehicle` implementation that the backend writes its results to.
This is defaulted to `VehicleImpl`, which uses atomics for the most important parameters.
If a parameter you want is not defined here, you can replace it with another class that provides the methods you need.

## Vehicle Backend

The framework features two user-selectable backends that handle requesting information from the vehicle.
The choice of backend affects how you connect the controller to the car and how information is requested.

- `ELMBackend` - uses serial communication with an ELM-compatible adapter, for example [this one](https://freematics.com/store/index.php?route=product/product&product_id=83)
- `CANBackend` - uses CAN communication with an adapter that exposes CAN bus line, for example [this one](https://www.amazon.com/iKKEGOL-Connector-Universal-Pigtail-Motorcycle/dp/B09QYFB7N8/ref=sr_1_5)

These are "selected" by overriding `auto vehicleBackend() -> VehicleBackend<>&` in `VehicleApplication`.
If `RGB_NATIVE` is enabled, this is not needed since a `MockBackend` will automatically be used.

Like `VehicleApplication<>`, `VehicleBackend<>` is also templated by the Vehicle implementation which defaults to `VehicleImpl`.
The parameters of the backend are templated as well. Those will be omitted below for readability.

### ELMBackend

The `ELMBackend` constructor takes the following parameters:

| Parameter            | Usage                                      |
|----------------------|--------------------------------------------|
| `PinNumber rx`       | Controller RX pin                          |
| `PinNumber tx`       | Controller TX pin                          |
| `span<ELMParameter>` | Configuration of what paramters to monitor |

The `ELMParameter` constructor takes the following parameters:

| Parameter                                | Usage                                                  |
|------------------------------------------|--------------------------------------------------------|
| `u8 pid`                                 | OBD-II Mode 1 SAE J1979 PID                            |
| `VehicleSetter<Vehicle> vehicleSetter{}` | Function that sets the result to the vehicle interface |

### CANBackend

The `CANBackend` constructor takes the following parameters:

| Parameter             | Usage                                      |
|-----------------------|--------------------------------------------|
| `PinNumber pinNumber` | SPI CS Pin connected to MCP2515            |
| `ClockRate clockRate` | MCP2515 clock rate                         |
| `BaudRate baudRate`   | CAN Bus BAUD Rate                          |
| `span<CANEcu> ecus`   | ECU ids to monitor                         |
| `span<CANParameter>`  | Configuration of what paramters to monitor |

The `CANParameter` constructor takes the following parameters:

| Parameter                                     | Usage                                                               |
|-----------------------------------------------|---------------------------------------------------------------------|
| `CANRequestMessage message`                   | The CAN Message template for the request                            |
| `Function<u32(const DataBuffer&)> dataMapper` | Function that converts the data buffer's result into a 32 bit value |
| `VehicleSetter<Vehicle> vehicleSetter{}`      | Function that sets the result to the vehicle interface              |
| `Duration frequency`                          | How often to query for this paramter                                |
| `u8 priority`                                 | Priority for MCP2515                                                |

The `CANRequestMessage` constructor takes the following parameters:

| Parameter                                | Usage                 |
|------------------------------------------|-----------------------|
| `u16 canId`                              | The CAN Message id    |
| `u16 pId`                                | Parameter id to query |
| `u8 mode`                                | Mode to query         |

## Instrumentation

If an SD card is detected, the Vehicle process will log vehicle data at a configurable interval to a log file on the SD card

The format is in binary, use `vlog-to-csv.py` to convert it to CSV for analysis

See Also
- `class VehicleLogger`
- `struct VehicleData`