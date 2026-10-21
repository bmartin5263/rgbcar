//
// Created by Brandon on 8/13/26.
//

#ifndef RGBCAR_VEHICLEEVENTS_H
#define RGBCAR_VEHICLEEVENTS_H

#include <EventType.h>
#include <SystemEvents.h>

#include "GearPosition.h"

namespace rgb::car {
struct VehicleConnected : BaseEvent {};
struct VehicleDisconnected : BaseEvent {};

struct GearPositionChanged : BaseEvent {};

struct GearNumberChanged : BaseEvent {};

struct BrakePressed : BaseEvent {};
struct BrakeReleased : BaseEvent {};

struct OverdriveActivated : BaseEvent {};
struct OverdriveDeactivated : BaseEvent {};

struct TractionControlActivated : BaseEvent {};
struct TractionControlDeactivated : BaseEvent {};

// TODO - lincoln specific
struct InfoButtonPressed : BaseEvent {};
struct SelectButtonPressed : BaseEvent {};

struct ParkingLightsActivated : BaseEvent {};
struct ParkingLightsDeactivated : BaseEvent {};

using VehicleEvents = Event<
  VehicleConnected,
  VehicleDisconnected,
  GearPositionChanged,
  GearNumberChanged,
  BrakePressed,
  BrakeReleased,
  OverdriveActivated,
  OverdriveDeactivated,
  TractionControlActivated,
  TractionControlDeactivated,
  InfoButtonPressed,
  SelectButtonPressed,
  ParkingLightsActivated,
  ParkingLightsDeactivated
>;

template<typename ...UserEvents>
using VehicleEvent = extend_variant_t<VehicleEvents, UserEvents...>;

}

#endif //RGBCAR_VEHICLEEVENTS_H
