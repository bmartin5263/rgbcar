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
    auto& vehicle = Vehicle::Instance();
    ledStrip.fill(Color::GREEN().lerpClamp(Color::RED(), vehicle.rpm() / 5000.f));
  }
};
```

## Vehicle Class

`Vehicle` is the main thread-safe interface used to read information from the vehicle that is written to by the vehicle backend on its separate core.

Each property is a separate atomic and their update schedule is configurable when creating the backend.

## Vehicle Backend

The framework features two user-selectable backends that handle requesting information from the vehicle.
The choice of backend affects how you connect the controller to the car and how information is requested.

- `ELMBackend` - uses serial communication with an ELM-compatible adapter, for example [this one](https://freematics.com/store/index.php?route=product/product&product_id=83)
- `CANBackend` - uses CAN communication with an adapter that exposes CAN bus line, for example [this one](https://www.amazon.com/iKKEGOL-Connector-Universal-Pigtail-Motorcycle/dp/B09QYFB7N8/ref=sr_1_5)

These are "selected" by overriding `auto vehicleBackend() -> VehicleBackend&` in `VehicleApplication`.
If `RGB_NATIVE` is enabled, this is not needed since a `MockBackend` will automatically be used.

### ELMBackend

The `ELMBackend` constructor takes the following parameters:

| Argument             | Usage                                    |
|----------------------|------------------------------------------|
| `PinNumber rx`       | Controller RX pin                        |
| `PinNumber tx`       | Controller TX pin                        |
| `span<ELMParameter>` | Configuration of what paramters to query |

## Instrumentation

If an SD card is detected, the Vehicle process will log vehicle data at a configurable interval to a log file on the SD card

The format is in binary, use `vlog-to-csv.py` to convert it to CSV for analysis

See Also
- `class VehicleLogger`
- `struct VehicleData`