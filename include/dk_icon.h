#if !defined(DK_ICON_H)
#define DK_ICON_H

#if !defined(SDL_h_)
#include <SDL2/SDL.h>
#endif

typedef enum icon_type
{
  ICON_NONE = 0,
  ICON_PLAY,
  ICON_PAUSE,
  ICON_GRID,
  ICON_EXIT,
  ICON_SAVE,
  ICON_CLEAR,
  ICON_COUNT
} icon_type_t;

typedef struct
{
  SDL_Rect rect;
  SDL_Texture* texture;
  icon_type_t type;
} icon_t;

void
icon_load_32x32(icon_t* icon, icon_type_t type, SDL_Renderer* renderer);

SDL_Texture*
icon_load__texture(SDL_Renderer* renderer, char* path);

#if defined(DK_ICON_IMPLEMENTATION)

SDL_Texture*
icon_load__texture(SDL_Renderer* renderer, char* path)
{
  SDL_Surface* surface = IMG_Load(path);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  return texture;
}

void
icon_load_32x32(icon_t* icon, icon_type_t type, SDL_Renderer* renderer)
{

  icon->type = type;

  icon->rect.w = 32;
  icon->rect.h = 32;

  icon->rect.x = 0;
  icon->rect.y = 0;

  switch (type)
  {
    case ICON_PLAY:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Play.png");
    } break;
    case ICON_PAUSE:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Pause.png");
    } break;
    case ICON_GRID:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Grid.png");
    } break;
    case ICON_EXIT:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Power.png");
    } break;
    case ICON_SAVE:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Floppy-Drive.png");
    } break;
    case ICON_CLEAR:
    {
      icon->texture = icon_load__texture(renderer, "assets/icons/Redo.png");
    } break;
    default:
    {
      icon->texture = NULL;
    } break;
  }
}

#endif // DK_ICON_IMPLEMENTATION


#endif // DK_ICON_H
