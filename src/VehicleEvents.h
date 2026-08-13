//
// Created by Brandon on 8/13/26.
//

#ifndef RGBCAR_VEHICLEEVENTS_H
#define RGBCAR_VEHICLEEVENTS_H

#include <EventType.h>
#include <SystemEvents.h>

namespace rgb::car {
struct VehicleConnected : BaseEvent {};
struct VehicleDisconnected : BaseEvent {};
struct CarEngineStarted : BaseEvent {};
struct CarEngineStopped : BaseEvent {};

using VehicleEvents = Event<
  VehicleConnected,
  VehicleDisconnected,
  CarEngineStarted,
  CarEngineStopped
>;

template<typename ...UserEvents>
using VehicleEvent = extend_variant_t<VehicleEvents, UserEvents...>;

}

#endif //RGBCAR_VEHICLEEVENTS_H
