#if !defined(DK_UI_H)
#define DK_UI_H

#include <SDL2/SDL.h>

#include "dk_icon.h"
#include "dk_color.h"
#include "dk_text.h"
#include "dk_app.h"

bool
dk_ui_button(app_t* game, SDL_Rect rect, SDL_Color color, char* text, bool* focused);

bool
dk_ui_icon_button(app_t* game, SDL_Rect rect, SDL_Color color, SDL_Texture* texture, bool* focused);

void
dk_ui_text_input(app_t* game, SDL_Point position, char* placeholder, void(callback)(char*), bool* focused);

void
dk_ui_tooltip(app_t* game, SDL_Rect rect, char* text, bool* visible);

#if defined(DK_UI_IMPLEMENTATION)

void
dk_ui_tooltip(app_t* game, SDL_Rect rect, char* text, bool* visible)
{

  if (rect.x < game->mouse.x && game->mouse.x < rect.x + rect.w && rect.y < game->mouse.y && game->mouse.y < rect.y + rect.h) {
    *visible = true;
  } else {
    *visible = false;
  }

  if (*visible) {

    int text_width = dk_text_width(&game->ui_text, text);
    int text_height = dk_text_height(&game->ui_text, text);

    SDL_Rect tooltip_rect = {
      rect.x + rect.w + 10,
      rect.y + rect.h / 2 - text_height / 2,
      text_width + 10,
      text_height + 10
    };

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(game->renderer, &tooltip_rect);

    SDL_Point position = {
      tooltip_rect.x + tooltip_rect.w / 2 - dk_text_width(&game->ui_text, text) / 2,
      tooltip_rect.y
    };

    game->ui_text.color = C64_WHITE;
    dk_text_draw(&game->ui_text, text, position.x, position.y);
  }
}

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

  char* text = "Hello, World";

  int text_width = dk_text_width(&game->ui_text, text);
  int text_height = dk_text_height(&game->ui_text, text);

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
  if (strlen(text) == 0) {
    dk_text_draw(&game->ui_text, placeholder, position.x + text_margin, position.y);
  } else {
    dk_text_draw(&game->ui_text, text, position.x + text_margin, position.y);
  }

  // Unerline Curso
  SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(game->renderer, &(SDL_Rect){ position.x + text_width + text_margin, position.y + text_height, 10, 5 });

  static int number_of_keys = 0;
  if (SDL_GetKeyboardState(&number_of_keys)[SDL_SCANCODE_RETURN]) {
    callback(text);
  }
}

#endif // DK_UI_IMPLEMENTATION


#endif // DK_UI_H
