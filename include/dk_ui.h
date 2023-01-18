#if !defined(DK_UI_H)
#define DK_UI_H

#if !defined(SDL_h_)
#include <SDL2/SDL.h>
#endif

#if !defined(DK_ICON_H)
#include "dk_icon.h"
#endif

#if !defined(DK_COLOR_H)
#include "dk_color.h"
#endif

#if !defined(DK_TEXT_H)
#include "dk_text.h"
#endif

#if !defined(DK_APP_H)
#include "dk_app.h"
#endif

bool
dk_ui_button(app_t* game, SDL_Rect rect, SDL_Color color, char* text, bool* focused);

bool
dk_ui_icon_button(app_t* game, SDL_Rect rect, SDL_Color color, SDL_Texture* texture, bool* focused);

void
dk_ui_text_input(app_t* game, SDL_Point position, char* placeholder, void(callback)(char*), bool* focused);

#if defined(DK_UI_IMPLEMENTATION)

bool
dk_ui_button(app_t* game, SDL_Rect rect, SDL_Color color, char* text, bool* focused)
{

  SDL_Color btn_color = color;

  int text_width = dk_text_width(&game->ui_text, text);
  int text_height = dk_text_height(&game->ui_text, text);

  if (rect.w < text_width) {
    rect.w = text_width + 10;
  }

  if (rect.h < text_height) {
    rect.h = text_height + 10;
  }

  int mouse_pressed = 0;

  if (rect.x < game->mouse.x && game->mouse.x < rect.x + rect.w && rect.y < game->mouse.y && game->mouse.y < rect.y + rect.h) {

    *focused = true;

    mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

    // Hover color
    btn_color.r = color.r * 0.5;
    btn_color.g = color.g * 0.5;
    btn_color.b = color.b * 0.5;
    btn_color.a = color.a * 0.5;

    // Button Pressed color
    if (mouse_pressed) {
      btn_color.r = color.r * 0.2;
      btn_color.g = color.g * 0.2;
      btn_color.b = color.b * 0.2;
      btn_color.a = color.a * 0.2;
    }

  } else {
    btn_color = color;
    *focused = false;
  }

  SDL_SetRenderDrawColor(game->renderer, btn_color.r, btn_color.g, btn_color.b, btn_color.a);
  SDL_RenderFillRect(game->renderer, &rect);

  SDL_Point position = {
    rect.x + rect.w / 2 - dk_text_width(&game->ui_text, text) / 2,
    rect.y + rect.h / 2 - dk_text_height(&game->ui_text, text) / 2
  };

  // BG color is dark, use white text
  SDL_Color text_color = C64_BLACK;
  if (btn_color.r < 128 && btn_color.g < 128 && btn_color.b < 128) {
    text_color = C64_WHITE;
  }

  game->ui_text.color = text_color;
  dk_text_draw(&game->ui_text, text, position.x, position.y);

  if (mouse_pressed) {
    return true;
  }

  return false;
}

bool dk_ui_icon_button(app_t* game, SDL_Rect rect, SDL_Color color, SDL_Texture* texture, bool* focused)
{

  SDL_Color btn_color = color;
  int mouse_pressed = 0;

  if (rect.x < game->mouse.x && game->mouse.x < rect.x + rect.w && rect.y < game->mouse.y && game->mouse.y < rect.y + rect.h) {

    *focused = true;
    mouse_pressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);

    // Hover color
    btn_color.r = color.r * 0.5;
    btn_color.g = color.g * 0.5;
    btn_color.b = color.b * 0.5;
    btn_color.a = color.a * 0.5;

    // Button Pressed color
    if (mouse_pressed) {
      btn_color.r = color.r * 0.2;
      btn_color.g = color.g * 0.2;
      btn_color.b = color.b * 0.2;
      btn_color.a = color.a * 0.2;
    }

  } else {
    btn_color = color;
    *focused = false;
  }

  SDL_Rect image_rect = {
    rect.x + rect.w / 2 - rect.w / 2,
    rect.y + rect.h / 2 - rect.h / 2,
    rect.w,
    rect.h
  };

  SDL_SetTextureColorMod(texture, btn_color.r, btn_color.g, btn_color.b);
  SDL_RenderCopy(game->renderer, texture, NULL, &image_rect);

  if (mouse_pressed) {
    return true;
  }

  return false;
}

void dk_ui_text_input(app_t* game, SDL_Point position, char* placeholder, void(callback)(char*), bool* focused) {

  int text_width = dk_text_width(&game->ui_text, game->raw_text_input_buffer);
  int text_height = dk_text_height(&game->ui_text, game->raw_text_input_buffer);

  SDL_Color color = C64_YELLOW;
  SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);

  SDL_Rect rect = { position.x, position.y, 200, 40 };

  if (rect.w < text_width) {
    rect.w = text_width + 10;
  }

  if (rect.h < text_height) {
    rect.h = text_height + 10;
  }

  if (rect.x < game->mouse.x && game->mouse.x < rect.x + rect.w && rect.y < game->mouse.y && game->mouse.y < rect.y + rect.h) {
    *focused = true;
  } else {
    *focused = false;
  }

  SDL_RenderFillRect(game->renderer, &rect);

  const int text_margin = 5;
  if (strlen(game->raw_text_input_buffer) == 0) {
    dk_text_draw(&game->ui_text, placeholder, position.x + text_margin, position.y);
  } else {
    dk_text_draw(&game->ui_text, game->raw_text_input_buffer, position.x + text_margin, position.y);
  }

  // Unerline Curso
  SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(game->renderer, &(SDL_Rect){ position.x + text_width + text_margin, position.y + text_height, 10, 5 });

  static int number_of_keys = 0;
  if (SDL_GetKeyboardState(&number_of_keys)[SDL_SCANCODE_RETURN]) {
    callback(game->raw_text_input_buffer);
  }
}

#endif // DK_UI_IMPLEMENTATION


#endif // DK_UI_H
