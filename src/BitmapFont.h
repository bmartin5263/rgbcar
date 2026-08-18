//
// Created by Brandon on 8/18/26.
//

#ifndef RGBCAR_BITMAPFONT_H
#define RGBCAR_BITMAPFONT_H

#if RGB_NATIVE

#include <SDL2/SDL.h>
#include <string_view>

namespace rgb::car {

// Minimal self-contained pixel font for the vehicle mock dashboard.
// Avoids pulling in SDL Fonts
class BitmapFont {
public:
  static constexpr auto GLYPH_COLUMNS = 4;
  static constexpr auto GLYPH_ROWS = 6;

  static auto DrawText(SDL_Renderer* renderer, std::string_view text, int x, int y, int pixelSize, SDL_Color color) -> void;
};

}

#endif //RGB_NATIVE

#endif //RGBCAR_BITMAPFONT_H
