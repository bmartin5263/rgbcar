//
// Created by Brandon on 8/18/26.
//

#include "VehicleControlPanel.h"

#if RGB_NATIVE

#include <cstdio>
#include <Pin.h>

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
constexpr auto SEGMENT_HEIGHT = 20;
constexpr auto STEPPER_BUTTON_SIZE = 24;
constexpr auto BIG_BUTTON_HEIGHT = 24;
constexpr auto ROW_HEIGHT = 30;
constexpr auto VALUE_WIDTH = 90;
constexpr auto TEXT_SCALE = 2;

auto PointInRect(int x, int y, const SDL_Rect& rect) -> bool {
  SDL_Point point{x, y};
  return SDL_PointInRect(&point, &rect);
}

auto TextWidth(std::string_view text, int pixelSize) -> int {
  if (text.empty()) {
    return 0;
  }
  return static_cast<int>(text.size()) * (BitmapFont::GLYPH_COLUMNS + 1) * pixelSize - pixelSize;
}

auto DrawCenteredText(SDL_Renderer* renderer, std::string_view text, const SDL_Rect& rect, int pixelSize, SDL_Color color) -> void {
  auto x = rect.x + (rect.w - TextWidth(text, pixelSize)) / 2;
  auto y = rect.y + (rect.h - BitmapFont::GLYPH_ROWS * pixelSize) / 2;
  BitmapFont::DrawText(renderer, text, x, y, pixelSize, color);
}

template<std::size_t N>
auto LayoutSegments(const SDL_Rect& bounds, std::array<SDL_Rect, N>& segments) -> void {
  auto segmentWidth = bounds.w / static_cast<int>(N);
  for (std::size_t i = 0; i < N; ++i) {
    auto x = bounds.x + static_cast<int>(i) * segmentWidth;
    auto width = (i + 1 == N) ? bounds.w - static_cast<int>(i) * segmentWidth : segmentWidth;
    segments[i] = SDL_Rect{x, bounds.y, width, bounds.h};
  }
}
}


auto VehicleControlPanel::initializeComponentLayout() -> void {
  auto row = 0;
  for (auto* slider : sliders()) {
    slider->track = SDL_Rect{MARGIN_X + LABEL_WIDTH, MARGIN_Y + row * ROW_HEIGHT, TRACK_WIDTH, TRACK_HEIGHT};
    ++row;
  }

  mBrakeButton.bounds = SDL_Rect{
    MARGIN_X + LABEL_WIDTH,
    MARGIN_Y + row * ROW_HEIGHT - BIG_BUTTON_HEIGHT / 4,
    LABEL_WIDTH + TRACK_WIDTH,
    BIG_BUTTON_HEIGHT
  };
  ++row;

  mGearPositionSegmented.bounds = SDL_Rect{
    MARGIN_X + LABEL_WIDTH,
    MARGIN_Y + row * ROW_HEIGHT,
    TRACK_WIDTH,
    SEGMENT_HEIGHT};
  LayoutSegments(mGearPositionSegmented.bounds, mGearPositionSegmented.segmentRects);
  ++row;

  auto stepperY = MARGIN_Y + row * ROW_HEIGHT;
  mGearNumberStepper.decrementButton = SDL_Rect{MARGIN_X + LABEL_WIDTH, stepperY, STEPPER_BUTTON_SIZE, STEPPER_BUTTON_SIZE};
  mGearNumberStepper.valueBox = SDL_Rect{
    MARGIN_X + LABEL_WIDTH + STEPPER_BUTTON_SIZE, stepperY,
    TRACK_WIDTH - 2 * STEPPER_BUTTON_SIZE, STEPPER_BUTTON_SIZE
  };
  mGearNumberStepper.incrementButton = SDL_Rect{
    MARGIN_X + LABEL_WIDTH + TRACK_WIDTH - STEPPER_BUTTON_SIZE, stepperY,
    STEPPER_BUTTON_SIZE, STEPPER_BUTTON_SIZE
  };
  ++row;

  for (auto* box : checkboxes()) {
    box->box = SDL_Rect{MARGIN_X + LABEL_WIDTH, MARGIN_Y + row * ROW_HEIGHT, CHECKBOX_SIZE, CHECKBOX_SIZE};
    ++row;
  }

  mInfoSelectSegmented.bounds = SDL_Rect{MARGIN_X + LABEL_WIDTH, MARGIN_Y + row * ROW_HEIGHT, TRACK_WIDTH, SEGMENT_HEIGHT};
  LayoutSegments(mInfoSelectSegmented.bounds, mInfoSelectSegmented.segmentRects);
  ++row;
}

auto VehicleControlPanel::ensureWindow() -> void {
  if (mWindow) {
    return;
  }

  initializeComponentLayout();

  constexpr auto EXTRA_ROWS = 4; // brake button, gear position segmented, gear number stepper, info/select segmented
  auto width = MARGIN_X * 2 + LABEL_WIDTH + TRACK_WIDTH + VALUE_WIDTH;
  auto height = MARGIN_Y * 2 + static_cast<int>(sliders().size() + checkboxes().size() + EXTRA_ROWS) * ROW_HEIGHT;
  mWindow.reset(SDL_CreateWindow(
    "Vehicle Mock Controls",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    width, height,
    SDL_WINDOW_SHOWN
  ));
  mRenderer.reset(SDL_CreateRenderer(mWindow.get(), -1, SDL_RENDERER_ACCELERATED));
}

auto VehicleControlPanel::handleInput() -> void {
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

    if (PointInRect(x, y, mBrakeButton.bounds)) {
      mBrakeButton.value = !mBrakeButton.value;
      mThrottleSlider.value = 0.f;
    }

    if (PointInRect(x, y, mGearNumberStepper.decrementButton)) {
      mGearNumberStepper.value = Clamp(mGearNumberStepper.value - 1, mGearNumberStepper.minValue, mGearNumberStepper.maxValue);
    }
    if (PointInRect(x, y, mGearNumberStepper.incrementButton)) {
      mGearNumberStepper.value = Clamp(mGearNumberStepper.value + 1, mGearNumberStepper.minValue, mGearNumberStepper.maxValue);
    }

    for (std::size_t i = 0; i < mGearPositionSegmented.segmentRects.size(); ++i) {
      if (PointInRect(x, y, mGearPositionSegmented.segmentRects[i])) {
        mGearPositionSegmented.selectedIndex = static_cast<int>(i);
        break;
      }
    }

    for (std::size_t i = 0; i < mInfoSelectSegmented.segmentRects.size(); ++i) {
      if (PointInRect(x, y, mInfoSelectSegmented.segmentRects[i])) {
        mInfoSelectSegmented.selected[i] = !mInfoSelectSegmented.selected[i];
      }
    }
  }

  if (leftDown && mActiveSlider != nullptr) {
    auto t = Clamp(static_cast<float>(x - mActiveSlider->track.x) / static_cast<float>(mActiveSlider->track.w), 0.f, 1.f);
    mActiveSlider->value = mActiveSlider->minValue + t * (mActiveSlider->maxValue - mActiveSlider->minValue);
  }

  mWasLeftDown = leftDown;
}

auto VehicleControlPanel::render() -> void {
  auto* renderer = mRenderer.get();

  // Color screen Gray
  SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
  SDL_RenderClear(renderer);

  for (auto* slider : sliders()) {
    drawSlider(*slider);
  }
  drawButton(mBrakeButton);
  drawSegmentedControl(mGearPositionSegmented);
  drawStepper(mGearNumberStepper);
  for (auto* box : checkboxes()) {
    drawCheckbox(*box);
  }
  drawMultiSegmentedControl(mInfoSelectSegmented);

  SDL_RenderPresent(renderer);
}

auto VehicleControlPanel::drawSlider(const SliderControl& slider) const -> void {
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

auto VehicleControlPanel::drawCheckbox(const CheckboxControl& box) const -> void {
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

auto VehicleControlPanel::drawButton(const ButtonControl& button) const -> void {
  auto renderer = mRenderer.get();

  if (button.value) {
    SDL_SetRenderDrawColor(renderer, 200, 60, 60, 255);
  }
  else {
    SDL_SetRenderDrawColor(renderer, 90, 40, 40, 255);
  }
  SDL_RenderFillRect(renderer, &button.bounds);
  DrawCenteredText(renderer, button.label, button.bounds, TEXT_SCALE, LABEL_COLOR);
}

auto VehicleControlPanel::drawStepper(const StepperControl& stepper) const -> void {
  auto renderer = mRenderer.get();
  BitmapFont::DrawText(renderer, stepper.label, MARGIN_X, stepper.decrementButton.y - 1, TEXT_SCALE, LABEL_COLOR);

  SDL_SetRenderDrawColor(renderer, 90, 90, 100, 255);
  SDL_RenderFillRect(renderer, &stepper.decrementButton);
  SDL_RenderFillRect(renderer, &stepper.incrementButton);
  DrawCenteredText(renderer, "DN", stepper.decrementButton, TEXT_SCALE, LABEL_COLOR);
  DrawCenteredText(renderer, "UP", stepper.incrementButton, TEXT_SCALE, LABEL_COLOR);

  SDL_SetRenderDrawColor(renderer, 60, 60, 66, 255);
  SDL_RenderFillRect(renderer, &stepper.valueBox);

  char buf[4];
  std::snprintf(buf, sizeof(buf), "%d", stepper.value);
  DrawCenteredText(renderer, buf, stepper.valueBox, TEXT_SCALE, LABEL_COLOR);
}

auto VehicleControlPanel::drawSegmentedControl(const SegmentedControl& segmented) const -> void {
  auto renderer = mRenderer.get();
  BitmapFont::DrawText(
    renderer, segmented.label, MARGIN_X,
    segmented.bounds.y + (segmented.bounds.h - BitmapFont::GLYPH_ROWS * TEXT_SCALE) / 2,
    TEXT_SCALE, LABEL_COLOR
  );

  for (std::size_t i = 0; i < segmented.segmentRects.size(); ++i) {
    const auto& rect = segmented.segmentRects[i];
    if (static_cast<int>(i) == segmented.selectedIndex) {
      SDL_SetRenderDrawColor(renderer, 80, 180, 120, 255);
    }
    else {
      SDL_SetRenderDrawColor(renderer, 60, 60, 66, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    DrawCenteredText(renderer, segmented.segmentLabels[i], rect, TEXT_SCALE, LABEL_COLOR);
  }

  SDL_SetRenderDrawColor(renderer, 20, 20, 24, 255);
  for (const auto& rect : segmented.segmentRects) {
    SDL_RenderDrawRect(renderer, &rect);
  }
}

auto VehicleControlPanel::drawMultiSegmentedControl(const MultiSegmentedControl& segmented) const -> void {
  auto renderer = mRenderer.get();
  BitmapFont::DrawText(
    renderer, segmented.label, MARGIN_X,
    segmented.bounds.y + (segmented.bounds.h - BitmapFont::GLYPH_ROWS * TEXT_SCALE) / 2,
    TEXT_SCALE, LABEL_COLOR
  );

  for (std::size_t i = 0; i < segmented.segmentRects.size(); ++i) {
    const auto& rect = segmented.segmentRects[i];
    if (segmented.selected[i]) {
      SDL_SetRenderDrawColor(renderer, 80, 180, 120, 255);
    }
    else {
      SDL_SetRenderDrawColor(renderer, 60, 60, 66, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    DrawCenteredText(renderer, segmented.segmentLabels[i], rect, TEXT_SCALE, LABEL_COLOR);
  }

  SDL_SetRenderDrawColor(renderer, 20, 20, 24, 255);
  for (const auto& rect : segmented.segmentRects) {
    SDL_RenderDrawRect(renderer, &rect);
  }
}

auto VehicleControlPanel::syncFromVehicle(const Vehicle& vehicle) -> void {
  mRpmSlider.value = static_cast<float>(vehicle.rpm());
  mSpeedSlider.value = static_cast<float>(ToMph(vehicle.speed()));
  mCoolantSlider.value = vehicle.coolantTemp();
  mFuelSlider.value = vehicle.fuelLevel() * 100.f;
  mThrottleSlider.value = vehicle.throttlePosition() * 100.f;

  mGearNumberStepper.value = vehicle.gearNumber();
  mGearPositionSegmented.selectedIndex = static_cast<int>(vehicle.gearPosition());

  mBrakeButton.value = vehicle.isBrakeApplied();
  mOverdriveCheckbox.value = vehicle.isOverdriveActive();
  mTCSCheckbox.value = vehicle.isTCSActive();
  mConnectedCheckbox.value = vehicle.isConnected();

  mInfoSelectSegmented.selected[0] = vehicle.isInfoButtonPressed();
  mInfoSelectSegmented.selected[1] = vehicle.isSelectButtonPressed();
}

auto VehicleControlPanel::applyToVehicle(Vehicle& vehicle) const -> void {
  vehicle.setRpm(static_cast<revs_per_minute>(mRpmSlider.value));
  vehicle.setSpeed(ToKph(static_cast<mph>(mSpeedSlider.value)));
  vehicle.setCoolantTemp(mCoolantSlider.value);
  vehicle.setFuelLevel(mFuelSlider.value / 100.f);
  vehicle.setThrottlePosition(mThrottleSlider.value / 100.f);

  vehicle.setGearNumber(static_cast<u8>(mGearNumberStepper.value));
  vehicle.setGearPosition(static_cast<GearPosition>(mGearPositionSegmented.selectedIndex));

  vehicle.setBrakeApplied(mBrakeButton.value);
  vehicle.setOverdriveActive(mOverdriveCheckbox.value);
  vehicle.setTCSActive(mTCSCheckbox.value);
  if (mConnectedCheckbox.value) {
    vehicle.setConnected(true);
  }
  else {
    vehicle.setConnected(false);
  }

  vehicle.setInfoButtonPressed(mInfoSelectSegmented.selected[0]);
  vehicle.setSelectButtonPressed(mInfoSelectSegmented.selected[1]);
}

auto VehicleControlPanel::update(Vehicle& vehicle) -> void {
  ensureWindow();
  syncFromVehicle(vehicle);
  handleInput();
  render();
  applyToVehicle(vehicle);
}

}

#endif //RGB_NATIVE
