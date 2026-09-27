//
// Created by Brandon on 8/18/26.
//

#ifndef RGBCAR_VEHICLEMOCKDASHBOARD_H
#define RGBCAR_VEHICLEMOCKDASHBOARD_H

#if RGB_NATIVE

#include <array>
#include <memory>
#include <SDL2/SDL.h>
#include "GearPosition.h"
#include "Vehicle.h"

namespace rgb::car {

class VehicleControlPanel {
  constexpr static auto LABEL_COLOR = SDL_Color{200, 200, 200, 255};
  constexpr static std::size_t SLIDER_COUNT = 5;
  constexpr static std::size_t CHECKBOX_COUNT = 3;
  constexpr static std::size_t GEAR_POSITION_COUNT = 5;
  constexpr static std::size_t INFO_SELECT_COUNT = 2;

  struct SliderControl {
    const char* label;
    const char* unit;
    float minValue;
    float maxValue;
    float value;
    SDL_Rect track{};
  };

  struct CheckboxControl {
    const char* label;
    SDL_Rect box{};
    bool value;
  };

  struct ButtonControl {
    const char* label;
    SDL_Rect bounds{};
    bool value;
  };

  struct StepperControl {
    const char* label;
    int minValue;
    int maxValue;
    int value;
    SDL_Rect decrementButton{};
    SDL_Rect valueBox{};
    SDL_Rect incrementButton{};
  };

  struct SegmentedControl {
    const char* label;
    std::array<const char*, GEAR_POSITION_COUNT> segmentLabels;
    SDL_Rect bounds{};
    std::array<SDL_Rect, GEAR_POSITION_COUNT> segmentRects{};
    int selectedIndex{0};
  };

  struct MultiSegmentedControl {
    const char* label;
    std::array<const char*, INFO_SELECT_COUNT> segmentLabels;
    SDL_Rect bounds{};
    std::array<SDL_Rect, INFO_SELECT_COUNT> segmentRects{};
    std::array<bool, INFO_SELECT_COUNT> selected{};
  };

public:
  VehicleControlPanel() = default;
  ~VehicleControlPanel() = default;
  VehicleControlPanel(const VehicleControlPanel& rhs) = delete;
  VehicleControlPanel(VehicleControlPanel&& rhs) noexcept = delete;
  VehicleControlPanel& operator=(const VehicleControlPanel& rhs) = delete;
  VehicleControlPanel& operator=(VehicleControlPanel&& rhs) noexcept = delete;

  // Creates the window on first call, reads mouse input, renders, and pushes
  // every control's current value into `vehicle`. Call once per frame.
  auto update(Vehicle& vehicle) -> void;

private:

  struct SDLWindowDeleter {
    auto operator()(SDL_Window* window) const -> void { SDL_DestroyWindow(window); }
  };
  struct SDLRendererDeleter {
    auto operator()(SDL_Renderer* renderer) const -> void { SDL_DestroyRenderer(renderer); }
  };

  auto ensureWindow() -> void;
  auto initializeComponentLayout() -> void;
  auto handleInput() -> void;
  auto render() -> void;
  auto syncFromVehicle(const Vehicle& vehicle) -> void;
  auto applyToVehicle(Vehicle& vehicle) const -> void;
  auto drawSlider(const SliderControl& slider) const -> void;
  auto drawCheckbox(const CheckboxControl& box) const -> void;
  auto drawButton(const ButtonControl& button) const -> void;
  auto drawStepper(const StepperControl& stepper) const -> void;
  auto drawSegmentedControl(const SegmentedControl& segmented) const -> void;
  auto drawMultiSegmentedControl(const MultiSegmentedControl& segmented) const -> void;

  std::unique_ptr<SDL_Window, SDLWindowDeleter> mWindow;
  std::unique_ptr<SDL_Renderer, SDLRendererDeleter> mRenderer;

  SliderControl mRpmSlider{"RPM", "", 0.f, 9999.f, 0.f};
  SliderControl mSpeedSlider{"SPD", "MPH", 0.f, 150.f, 0.f};
  SliderControl mCoolantSlider{"TMP", "F", 80.f, 200.f, 80.f};
  SliderControl mFuelSlider{"FUEL", "%", 0.f, 100.f, 0.f};
  SliderControl mThrottleSlider{"THR", "%", 0.f, 100.f, 0.f};

  ButtonControl mBrakeButton{"BRAKE", {}, false};

  SegmentedControl mGearPositionSegmented{"TRN", {"P", "R", "N", "D", "L"}};
  StepperControl mGearNumberStepper{"GEAR", 1, 8, 1};

  CheckboxControl mOverdriveCheckbox{"OD", {}, false};
  CheckboxControl mTCSCheckbox{"TCS", {}, false};
  CheckboxControl mConnectedCheckbox{"CONN", {}, false};

  MultiSegmentedControl mInfoSelectSegmented{"CTL", {"INFO", "SELECT"}};

  SliderControl* mActiveSlider{nullptr};
  bool mWasLeftDown{false};

  auto sliders() -> auto {
    return std::array{ &mRpmSlider, &mSpeedSlider, &mCoolantSlider, &mFuelSlider, &mThrottleSlider };
  }

  auto checkboxes() -> auto {
    return std::array{ &mOverdriveCheckbox, &mTCSCheckbox, &mConnectedCheckbox };
  }
};

}

#endif //RGB_NATIVE

#endif //RGBCAR_VEHICLEMOCKDASHBOARD_H
