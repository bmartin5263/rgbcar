//
// Created by Brandon on 8/18/26.
//

#ifndef RGBCAR_VEHICLEMOCKDASHBOARD_H
#define RGBCAR_VEHICLEMOCKDASHBOARD_H

#if RGB_NATIVE

#include <array>
#include <memory>
#include <SDL2/SDL.h>
#include "VehicleMock.h"

namespace rgb::car {

// Standalone SDL window with sliders/checkboxes for driving a VehicleMock by
// hand during native development.
class VehicleMockDashboard {
  constexpr static auto LABEL_COLOR = SDL_Color{200, 200, 200, 255};
  constexpr static std::size_t SLIDER_COUNT = 5;
  constexpr static std::size_t CHECKBOX_COUNT = 1;

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

public:
  VehicleMockDashboard() = default;
  ~VehicleMockDashboard() = default;
  VehicleMockDashboard(const VehicleMockDashboard& rhs) = delete;
  VehicleMockDashboard(VehicleMockDashboard&& rhs) noexcept = delete;
  VehicleMockDashboard& operator=(const VehicleMockDashboard& rhs) = delete;
  VehicleMockDashboard& operator=(VehicleMockDashboard&& rhs) noexcept = delete;

  // Creates the window on first call, reads mouse input, renders, and pushes
  // every control's current value into `vehicle`. Call once per frame.
  auto update(VehicleMock& vehicle) -> void;

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
  auto syncFromVehicle(const VehicleMock& vehicle) -> void;
  auto applyToVehicle(VehicleMock& vehicle) const -> void;
  auto drawSlider(const SliderControl& slider) const -> void;
  auto drawCheckbox(const CheckboxControl& box) const -> void;

  std::unique_ptr<SDL_Window, SDLWindowDeleter> mWindow;
  std::unique_ptr<SDL_Renderer, SDLRendererDeleter> mRenderer;

  SliderControl mRpmSlider{"RPM", "", 0.f, 9999.f, 0.f};
  SliderControl mSpeedSlider{"SPD", "MPH", 0.f, 150.f, 0.f};
  SliderControl mCoolantSlider{"TMP", "F", 80.f, 200.f, 80.f};
  SliderControl mFuelSlider{"FUEL", "%", 0.f, 100.f, 0.f};
  SliderControl mThrottleSlider{"THR", "%", 0.f, 100.f, 0.f};
  CheckboxControl mConnectedCheckbox{"CONN", {}, false};

  SliderControl* mActiveSlider{nullptr};
  bool mWasLeftDown{false};

  auto sliders() -> auto {
    return std::array{ &mRpmSlider, &mSpeedSlider, &mCoolantSlider, &mFuelSlider, &mThrottleSlider };
  }

  auto checkboxes() -> auto {
    return std::array{ &mConnectedCheckbox };
  }
};

}

#endif //RGB_NATIVE

#endif //RGBCAR_VEHICLEMOCKDASHBOARD_H
