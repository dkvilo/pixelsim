#if !defined(DK_COLOR_H)
#define DK_COLOR_H

#if !defined(SDL_h_)
#include <SDL2/SDL.h>
#endif

#define C64_BLACK (SDL_Color){ 0, 0, 0, 255 }
#define C64_WHITE (SDL_Color){ 255, 255, 255, 255 }
#define C64_RED (SDL_Color){ 136, 0, 0, 255 }
#define C64_CYAN (SDL_Color){ 170, 255, 238, 255 }
#define C64_PURPLE (SDL_Color){ 204, 68, 204, 255 }
#define C64_GREEN (SDL_Color){ 0, 204, 85, 255 }
#define C64_BLUE (SDL_Color){ 0, 0, 170, 255 }
#define C64_YELLOW (SDL_Color){ 238, 238, 119, 255 }
#define C64_ORANGE (SDL_Color){ 221, 136, 85, 255 }
#define C64_BROWN (SDL_Color){ 102, 68, 0, 255 }
#define C64_LIGHT_RED (SDL_Color){ 255, 119, 119, 255 }
#define C64_DARK_GREY (SDL_Color){ 51, 51, 51, 255 }
#define C64_GREY (SDL_Color){ 119, 119, 119, 255 }
#define C64_LIGHT_GREEN (SDL_Color){ 170, 255, 102, 255 }
#define C64_LIGHT_BLUE (SDL_Color){ 0, 136, 255, 255 }
#define C64_LIGHT_GREY (SDL_Color){ 187, 187, 187, 255 }
#define C64_COLOR_COUNT 16

void
load_colors(SDL_Color* buffer);

#if defined(DK_COLOR_IMPLEMENTATION)

SDL_Color C64_COLORS[C64_COLOR_COUNT] = {
  C64_BLACK,
  C64_WHITE,
  C64_RED,
  C64_CYAN,
  C64_PURPLE,
  C64_GREEN,
  C64_BLUE,
  C64_YELLOW,
  C64_ORANGE,
  C64_BROWN,
  C64_LIGHT_RED,
  C64_DARK_GREY,
  C64_GREY,
  C64_LIGHT_GREEN,
  C64_LIGHT_BLUE,
  C64_LIGHT_GREY,
};

void
load_colors(SDL_Color* buffer)
{
  buffer[0] = C64_BLACK;
  buffer[1] = C64_WHITE;
  buffer[2] = C64_RED;
  buffer[3] = C64_CYAN;
  buffer[4] = C64_PURPLE;
  buffer[5] = C64_GREEN;
  buffer[6] = C64_BLUE;
  buffer[7] = C64_YELLOW;
  buffer[8] = C64_ORANGE;
  buffer[9] = C64_BROWN;
  buffer[10] = C64_LIGHT_RED;
  buffer[11] = C64_DARK_GREY;
  buffer[12] = C64_GREY;
  buffer[13] = C64_LIGHT_GREEN;
  buffer[14] = C64_LIGHT_BLUE;
  buffer[15] = C64_LIGHT_GREY;
}

#endif // DK_COLOR_IMPLEMENTATION
#endif // DK_COLOR_H
