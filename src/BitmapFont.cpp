//
// Created by Brandon on 8/18/26.
//

#include "BitmapFont.h"

#if RGB_NATIVE

#include <array>
#include <cctype>
#include "Types.h"

namespace rgb::car {

namespace {

// Each glyph is GLYPH_ROWS rows of GLYPH_COLUMNS bits; bit 3 is the
// leftmost pixel, bit 0 the rightmost.
auto GlyphRows(char c) -> std::array<u8, BitmapFont::GLYPH_ROWS> {
  switch (c) {
    case '0': return {0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110};
    case '1': return {0b0010, 0b0110, 0b0010, 0b0010, 0b0010, 0b0111};
    case '2': return {0b0110, 0b1001, 0b0010, 0b0100, 0b1000, 0b1111};
    case '3': return {0b0110, 0b1001, 0b0010, 0b0001, 0b1001, 0b0110};
    case '4': return {0b0001, 0b0011, 0b0101, 0b1111, 0b0001, 0b0001};
    case '5': return {0b1111, 0b1000, 0b1110, 0b0001, 0b1001, 0b0110};
    case '6': return {0b0110, 0b1000, 0b1110, 0b1001, 0b1001, 0b0110};
    case '7': return {0b1111, 0b0001, 0b0010, 0b0100, 0b0100, 0b0100};
    case '8': return {0b0110, 0b1001, 0b0110, 0b1001, 0b1001, 0b0110};
    case '9': return {0b0110, 0b1001, 0b1001, 0b0111, 0b0001, 0b0110};
    case '%': return {0b1001, 0b0001, 0b0010, 0b0100, 0b1000, 0b1001};
    case 'R': return {0b1110, 0b1001, 0b1110, 0b1010, 0b1001, 0b1001};
    case 'P': return {0b1110, 0b1001, 0b1110, 0b1000, 0b1000, 0b1000};
    case 'M': return {0b1001, 0b1101, 0b1011, 0b1001, 0b1001, 0b1001};
    case 'S': return {0b1111, 0b1000, 0b1110, 0b0001, 0b1001, 0b0110};
    case 'D': return {0b1110, 0b1001, 0b1001, 0b1001, 0b1001, 0b1110};
    case 'T': return {0b1111, 0b0110, 0b0110, 0b0110, 0b0110, 0b0110};
    case 'H': return {0b1001, 0b1001, 0b1111, 0b1001, 0b1001, 0b1001};
    case 'F': return {0b1111, 0b1000, 0b1110, 0b1000, 0b1000, 0b1000};
    case 'U': return {0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110};
    case 'E': return {0b1111, 0b1000, 0b1110, 0b1000, 0b1000, 0b1111};
    case 'L': return {0b1000, 0b1000, 0b1000, 0b1000, 0b1000, 0b1111};
    case 'C': return {0b0111, 0b1000, 0b1000, 0b1000, 0b1000, 0b0111};
    case 'O': return {0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110};
    case 'N': return {0b1001, 0b1101, 0b1011, 0b1001, 0b1001, 0b1001};
    case 'W': return {0b1001, 0b1001, 0b1001, 0b1011, 0b1101, 0b1001};
    default: return {0, 0, 0, 0, 0, 0}; // space and anything unsupported render blank
  }
}

}

auto BitmapFont::DrawText(SDL_Renderer* renderer, std::string_view text, int x, int y, int pixelSize, SDL_Color color) -> void {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

  auto cursorX = x;
  for (auto c : text) {
    auto rows = GlyphRows(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    for (auto row = 0; row < GLYPH_ROWS; ++row) {
      for (auto col = 0; col < GLYPH_COLUMNS; ++col) {
        if ((rows[row] >> (GLYPH_COLUMNS - 1 - col)) & 0x1) {
          SDL_Rect rect{cursorX + col * pixelSize, y + row * pixelSize, pixelSize, pixelSize};
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }
    cursorX += (GLYPH_COLUMNS + 1) * pixelSize;
  }
}

}

#endif //RGB_NATIVE
