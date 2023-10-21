#if !defined(DK_TEXT_H)
#define DK_TEXT_H

#include "dk.h"

#if !defined(SDL_h_)
#include <SDL2/SDL.h>
#endif // SDL_h_

#if !defined(SDL_TTF_H_)
#include <SDL2/SDL_ttf.h>
#endif // SDL_TTF_H_

typedef struct
{
  SDL_Renderer* renderer;
  TTF_Font* font;
  SDL_Color color;
} dk_text_t;

void
dk_text_init(dk_text_t* draw_text,
             SDL_Renderer* renderer,
             TTF_Font* font,
             SDL_Color color);

void
dk_text_draw(dk_text_t* draw_text, char* text, i32 x, i32 y);

int
dk_text_width(dk_text_t* draw_text, char* text);

int
dk_text_height(dk_text_t* draw_text, char* text);

void
dk_text_set_size(dk_text_t *fptr, i32 size);

void
dk_text_destroy(dk_text_t* draw_text);

void
dk_text_draw_ext(dk_text_t* draw_text, char* text, i32 x, i32 y, SDL_Color color);

#if defined(DK_TEXT_IMPLEMENTATION)
void
dk_text_init(dk_text_t* draw_text,
             SDL_Renderer* renderer,
             TTF_Font* font,
             SDL_Color color)
{
  draw_text->renderer = renderer;
  draw_text->font = font;
  draw_text->color = color;
}

char*
dk_text_itoa(u32 number)
{
  char* text = malloc(10);
  sprintf(text, "%d", number);
  return text;
}

void dk_text_set_size(dk_text_t *fptr, i32 size) {
  TTF_SetFontSize(fptr->font, size);
}

void
dk_text_draw(dk_text_t* draw_text, char* text, i32 x, i32 y)
{
  dk_text_draw_ext(draw_text, text, x, y, draw_text->color);
}

void
dk_text_draw_ext(dk_text_t* draw_text, char* text, i32 x, i32 y, SDL_Color color)
{
  SDL_Surface* surface = TTF_RenderText_Solid(draw_text->font, text, color);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(draw_text->renderer, surface);
  SDL_Rect rect = { x, y, surface->w, surface->h };
  SDL_RenderCopy(draw_text->renderer, texture, NULL, &rect);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

int
dk_text_width(dk_text_t* draw_text, char* text)
{
  int w;
  TTF_SizeText(draw_text->font, text, &w, NULL);
  return w;
}

int
dk_text_height(dk_text_t* draw_text, char* text)
{
  int h;
  TTF_SizeText(draw_text->font, text, NULL, &h);
  return h;
}

void
dk_text_destroy(dk_text_t* draw_text)
{
  TTF_CloseFont(draw_text->font);
}

#endif // DK_TEXT_IMPLEMENTATION

#endif // DK_TEXT_H
