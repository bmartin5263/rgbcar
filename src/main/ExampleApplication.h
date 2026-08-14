//
// Created by Brandon on 10/11/25.
//

#ifndef RGBLIB_EXAMPLEAPPLICATION_H
#define RGBLIB_EXAMPLEAPPLICATION_H

#include "UserApplication.h"
#include "FastLEDMatrix.h"
#include "FastLEDStrip.h"
#include "IRReceiver.h"
#include "ReversePixelList.h"
#include "ChaseEffect.h"
#include "PixelSlice.h"

using namespace rgb;

// Set up the LED Strip and Sensors
inline auto ledCircuit = FastLEDStrip<38, D4_RGB>();
inline auto irRemote = IRReceiver{PinNumber{A7}};

// Effects
inline auto chaseEffect = ChaseEffect{};     // A chase effect template, can be applied to multiple segments simultaneously
inline auto effectHandle = EffectHandle{};   // Resource handle to control the running effect, not strictly needed

// Segments carved out from the primary circuit
inline auto leftSide = ledCircuit.slice(16);
inline auto rightSide = ledCircuit.slice(16, 38);
inline auto leftSideReversed = ReversePixelList{leftSide};
inline auto chaseGroup = std::array<PixelList*, 2> { &leftSideReversed, &rightSide };

struct MyCustomEvent : BaseEvent {};
using MyAppEvents = Event<MyCustomEvent>;

class ExampleApplication : public UserApplication<MyAppEvents> {
protected:
  auto configure(Configurer& app) -> void override {
    app.addLEDs(ledCircuit);   // Register the circuit to the app
    app.addSensor(irRemote);   // Register the IR Remote to the app

    // Defines what color the wipe effect should apply to each pixel
    chaseEffect.shader = [](auto currentColor, auto& params){
      return Color::GREEN() * params.positionRatio;
    };
    // 2 seconds for a full cycle
    chaseEffect.progression = EffectProgression::ConstantTime(Duration::Seconds(1));

    // Start the effect on the group
    effectHandle = Effects::Start(chaseEffect, chaseGroup);

    // Configure Event Handlers
    app.on<IRButtonPressed>([](auto& event) {
      if (event.button == IRButtonType::BUTTON_OK) {
        PublishEvent(MyCustomEvent{Clock::Now()});  // Publish an event
      }
    });
    app.on<MyCustomEvent>([](auto& event) {
      effectHandle.stop();  // Stop the effect
    });
  }
};


#endif //RGBLIB_EXAMPLEAPPLICATION_H
