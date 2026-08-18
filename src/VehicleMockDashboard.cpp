//
// Created by Brandon on 8/18/26.
//

#include "VehicleMockDashboard.h"

#if RGB_NATIVE

#include <cstdio>
#include "BitmapFont.h"
#include "Util.h"

namespace rgb::car {

namespace {
constexpr auto MARGIN_X = 12;
constexpr auto MARGIN_Y = 12;
constexpr auto LABEL_WIDTH = 50;
constexpr auto TRACK_WIDTH = 140;
constexpr auto TRACK_HEIGHT = 14;
constexpr auto CHECKBOX_SIZE = 16;
constexpr auto ROW_HEIGHT = 30;
constexpr auto VALUE_WIDTH = 90;
constexpr auto TEXT_SCALE = 2;

auto PointInRect(int x, int y, const SDL_Rect& rect) -> bool {
  SDL_Point point{x, y};
  return SDL_PointInRect(&point, &rect);
}
}


auto VehicleMockDashboard::initializeComponentLayout() -> void {
  auto row = 0;
  for (auto* slider : sliders()) {
    slider->track = SDL_Rect{MARGIN_X + LABEL_WIDTH, MARGIN_Y + row * ROW_HEIGHT, TRACK_WIDTH, TRACK_HEIGHT};
    ++row;
  }
  for (auto* box : checkboxes()) {
    box->box = SDL_Rect{MARGIN_X + LABEL_WIDTH, MARGIN_Y + row * ROW_HEIGHT, CHECKBOX_SIZE, CHECKBOX_SIZE};
    ++row;
  }
}

auto VehicleMockDashboard::ensureWindow() -> void {
  if (mWindow) {
    return;
  }

  initializeComponentLayout();

  auto width = MARGIN_X * 2 + LABEL_WIDTH + TRACK_WIDTH + VALUE_WIDTH;
  auto height = MARGIN_Y * 2 + static_cast<int>(sliders().size() + checkboxes().size()) * ROW_HEIGHT;
  mWindow.reset(SDL_CreateWindow(
    "Vehicle Mock Controls",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    width, height,
    SDL_WINDOW_SHOWN
  ));
  mRenderer.reset(SDL_CreateRenderer(mWindow.get(), -1, SDL_RENDERER_ACCELERATED));
}

auto VehicleMockDashboard::handleInput() -> void {
  auto focused = SDL_GetMouseFocus() == mWindow.get();
  auto x = 0;
  auto y = 0;
  auto buttons = focused ? SDL_GetMouseState(&x, &y) : Uint32{0};
  auto leftDown = focused && (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  auto justPressed = leftDown && !mWasLeftDown;

  if (!leftDown) {
    mActiveSlider = nullptr;
  }

  if (justPressed) {
    for (auto* slider : sliders()) {
      if (PointInRect(x, y, slider->track)) {
        mActiveSlider = slider;
        break;
      }
    }
    for (auto* box : checkboxes()) {
      if (PointInRect(x, y, box->box)) {
        box->value = !box->value;
      }
    }
  }

  if (leftDown && mActiveSlider != nullptr) {
    auto t = Clamp(static_cast<float>(x - mActiveSlider->track.x) / static_cast<float>(mActiveSlider->track.w), 0.f, 1.f);
    mActiveSlider->value = mActiveSlider->minValue + t * (mActiveSlider->maxValue - mActiveSlider->minValue);
  }

  mWasLeftDown = leftDown;
}

auto VehicleMockDashboard::render() -> void {
  auto* renderer = mRenderer.get();

  // Color screen Gray
  SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
  SDL_RenderClear(renderer);

  for (auto* slider : sliders()) {
    drawSlider(*slider);
  }
  for (auto* box : checkboxes()) {
    drawCheckbox(*box);
  }

  SDL_RenderPresent(renderer);
}

auto VehicleMockDashboard::drawSlider(const SliderControl& slider) const -> void {
  auto renderer = mRenderer.get();
  BitmapFont::DrawText(renderer, slider.label, MARGIN_X, slider.track.y - 1, TEXT_SCALE, LABEL_COLOR);

  SDL_SetRenderDrawColor(renderer, 60, 60, 66, 255);
  SDL_RenderFillRect(renderer, &slider.track);

  auto t = Clamp((slider.value - slider.minValue) / (slider.maxValue - slider.minValue), 0.f, 1.f);
  auto fill = slider.track;
  fill.w = static_cast<int>(static_cast<float>(slider.track.w) * t);
  SDL_SetRenderDrawColor(renderer, 80, 180, 120, 255);
  SDL_RenderFillRect(renderer, &fill);

  char buf[24];
  std::snprintf(buf, sizeof(buf), "%d%s", static_cast<int>(slider.value), slider.unit);
  BitmapFont::DrawText(renderer, buf, slider.track.x + slider.track.w + 10, slider.track.y - 1, TEXT_SCALE, LABEL_COLOR);
}

auto VehicleMockDashboard::drawCheckbox(const CheckboxControl& box) const -> void {
  auto renderer = mRenderer.get();
  BitmapFont::DrawText(renderer, box.label, MARGIN_X, box.box.y, TEXT_SCALE, LABEL_COLOR);

  if (box.value) {
    SDL_SetRenderDrawColor(renderer, 80, 180, 120, 255);
  }
  else {
    SDL_SetRenderDrawColor(renderer, 60, 60, 66, 255);
  }
  SDL_RenderFillRect(renderer, &box.box);

  BitmapFont::DrawText(renderer, box.value ? "ON" : "OFF", box.box.x + box.box.w + 10, box.box.y, TEXT_SCALE, LABEL_COLOR);
}

auto VehicleMockDashboard::applyToVehicle(VehicleMock& vehicle) const -> void {
  vehicle.setRpm(static_cast<revs_per_minute>(mRpmSlider.value));
  vehicle.setSpeed(ToKph(static_cast<mph>(mSpeedSlider.value)));
  vehicle.setCoolantTemp(mCoolantSlider.value);
  vehicle.setFuelLevel(mFuelSlider.value / 100.f);
  vehicle.setThrottlePosition(mThrottleSlider.value / 100.f);
  if (mConnectedCheckbox.value) {
    vehicle.connect(PinNumber{0}, PinNumber{0});
  }
  else {
    vehicle.disconnect();
  }
}

auto VehicleMockDashboard::update(VehicleMock& vehicle) -> void {
  ensureWindow();
  handleInput();
  render();
  applyToVehicle(vehicle);
}

}

#endif //RGB_NATIVE
